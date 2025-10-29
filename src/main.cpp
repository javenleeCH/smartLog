#include <iostream>
#include <memory>  // 新增：支持 std::make_unique（C++14+ 标准库）
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "llvm/ADT/StringRef.h"

// 基础命名空间（只引入必要的类，避免冲突）
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
// 修复1：llvm::cl 是命名空间，需用 using namespace 引入（不能用 using declaration）
using namespace llvm::cl;
// 修复2：LLVM 16 无 make_unique，用 C++ 标准库的 std::make_unique
using std::make_unique;
// 其他 llvm 类的引入（保持不变）
using llvm::errs;
using llvm::outs;
using llvm::Expected;
using llvm::handleAllErrors;
using llvm::ErrorInfoBase;
using llvm::StringRef;

// 修复3：现在 cl 命名空间已引入，可直接用 cl::OptionCategory
static OptionCategory LogAnnotatorCategory("log-annotator options");

class LogAnnotator : public MatchFinder::MatchCallback {
public:
  explicit LogAnnotator(Rewriter &R) : Rewrite(R) {}

  void run(const MatchFinder::MatchResult &Result) override {
    // 显式用 clang::StringLiteral 避免命名冲突
    if (const auto *Str = Result.Nodes.getNodeAs<clang::StringLiteral>("func_log_str")) {
      if (isTargetString(Str)) {
        if (const auto *Call = Result.Nodes.getNodeAs<CallExpr>("log_call")) {
          annotateStatement(Call->getSourceRange(), Result.Context);
        }
      }
    }

    if (const auto *Str = Result.Nodes.getNodeAs<clang::StringLiteral>("stream_log_str")) {
      if (isTargetString(Str)) {
        if (const auto *StreamCall = Result.Nodes.getNodeAs<CallExpr>("log_stream")) {
          annotateStatement(StreamCall->getSourceRange(), Result.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;

  bool isTargetString(const clang::StringLiteral *Str) {
    StringRef Content = Str->getString();
    return Content.find("pengtest") != StringRef::npos;
  }

  void annotateStatement(SourceRange Range, ASTContext *Context) {
    SourceManager &SM = Context->getSourceManager();
    const LangOptions &LangOpts = Context->getLangOpts();

    if (!Range.isValid() || SM.isInSystemHeader(Range.getBegin())) {
      return;
    }

    CharSourceRange StartRange = CharSourceRange::getCharRange(
        Range.getBegin(), 
        Range.getBegin().getLocWithOffset(2)
    );
    StringRef Code = Lexer::getSourceText(StartRange, SM, LangOpts);
    if (Code.startswith("//")) {
      return;
    }

    Rewrite.InsertText(Range.getBegin(), "// ");
    errs() << "注释了包含 'pengtest' 的语句: " 
           << Lexer::getSourceText(CharSourceRange::getTokenRange(Range), SM, LangOpts) 
           << "\n";
  }
};

class LogAnnotatorAction : public ASTFrontendAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef File) override {
    // 修复2：用 std::make_unique 初始化 Rewriter（LLVM 无 make_unique）
    Rewrite = make_unique<Rewriter>(CI.getSourceManager(), CI.getLangOpts());
    Annotator = make_unique<LogAnnotator>(*Rewrite);

    // 配置匹配器
    Finder.addMatcher(
        callExpr(
            hasAnyArgument(stringLiteral().bind("func_log_str"))
        ).bind("log_call"),
        Annotator.get()
    );

    Finder.addMatcher(
        callExpr(
            callee(functionDecl(hasName("operator<<"))),
            hasArgument(1, stringLiteral().bind("stream_log_str")),
            hasArgument(0, declRefExpr(to(varDecl(hasAnyName("cout", "cerr", "clog")))))
        ).bind("log_stream"),
        Annotator.get()
    );

    return Finder.newASTConsumer();
  }

  void EndSourceFileAction() override {
    if (Rewrite && Rewrite->overwriteChangedFiles()) {
      outs() << "已更新文件: " << getCurrentFile() << "\n";
    }
  }

private:
  std::unique_ptr<Rewriter> Rewrite;
  std::unique_ptr<LogAnnotator> Annotator;
  MatchFinder Finder;
};

int main(int argc, const char **argv) {
  Expected<CommonOptionsParser> OptionsParser = CommonOptionsParser::create(argc, argv, LogAnnotatorCategory);
  if (!OptionsParser) {
    handleAllErrors(OptionsParser.takeError(), [](const ErrorInfoBase &E) {
      errs() << "命令行参数解析失败: " << E.message() << "\n";
    });
    return 1;
  }

  ClangTool Tool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList());
  int Result = Tool.run(newFrontendActionFactory<LogAnnotatorAction>().get());
  if (Result != 0) {
    errs() << "工具运行失败\n";
    return Result;
  }

  return 0;
}