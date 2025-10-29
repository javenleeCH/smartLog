# smartLog - LLVM 示例项目

这是一个使用 LLVM 库的 CMake 项目示例。

## 项目结构

```
smartLog/
├── CMakeLists.txt  # CMake 构建配置文件
├── src/
│   └── main.cpp    # 主程序文件
└── README.md       # 项目说明
```

## 构建步骤

1. 确保已安装 LLVM 开发库

2. 创建构建目录
   ```bash
   mkdir build && cd build
   ```

3. 运行 CMake 配置
   ```bash
   cmake ..
   ```

4. 编译项目
   ```bash
   make
   ```

5. 运行程序
   ```bash
   ./smartLog
   ```

## 功能说明

当前示例程序创建了一个简单的 LLVM 模块，并生成了一个空的 main 函数的 IR 代码，然后将 IR 代码打印到标准输出。

## 扩展提示

- 要添加更多 LLVM 功能，可以在 `CMakeLists.txt` 中修改 `llvm_map_components_to_libnames` 来包含更多组件
- 在 `main.cpp` 中可以使用 LLVM API 来进行代码分析、转换或生成