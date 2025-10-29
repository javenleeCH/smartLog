cmake_minimum_required(VERSION 3.14)
project(smartLog)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找LLVM包
find_package(LLVM REQUIRED CONFIG)
get_filename_component(LLVM_CMAKE_ROOT ${LLVM_DIR} DIRECTORY)
set(CLANG_CMAKE_DIR "${LLVM_CMAKE_ROOT}/clang")
message(STATUS "pengtest ${CLANG_CMAKE_DIR}")


find_package(Clang REQUIRED CONFIG PATHS ${CLANG_CMAKE_DIR} NO_DEFAULT_PATH)

# 输出调试信息（确认找到的 LLVM/Clang 路径是否正确）
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM 库路径: ${LLVM_LIBRARY_DIRS}")
message(STATUS "Found Clang ${CLANG_VERSION}")
message(STATUS "Clang 库路径: ${CLANG_LIBRARY_DIRS}")
message(STATUS "Clang 自动识别的库: ${CLANG_LIBS}")


# 添加LLVM包含目录
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


# 查找必要的LLVM组件
llvm_map_components_to_libnames(llvm_libs 
  clangTooling
  clangASTMatchers
  clangRewrite
  clangFrontend
  clangSerialization
  clangAST
  clangBasic
  clangLex
  LLVMOption
  LLVMSupport)

# 创建原始的LLVM示例可执行文件
add_executable(smartLog src/main.cpp)

target_link_directories(smartLog PRIVATE ${LLVM_LIBRARY_DIRS})


# 链接LLVM库
target_link_libraries(smartLog  clangTooling
  clangASTMatchers
  clangRewrite
  clangFrontend
  clangSerialization
  clangAST
  clangBasic
  clangLex)


  add_executable(testxx src/xx.cpp)
  target_include_directories(testxx PRIVATE ${LLVM_INCLUDE_DIRS})