# Drawer-Linux

技术栈：C++20、Qt 6、Qt Widgets、CMake、Linux

运行环境：Debian 13、KDE Plasma 6、Wayland

安装构建依赖：

```bash
sudo apt install cmake qt6-base-dev qt6-base-dev-tools libxkbcommon-dev libssl-dev
```

构建Debug版本：

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j$(nproc)
ctest --test-dir build-debug --output-on-failure
```

构建Release版本：

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j$(nproc)
ctest --test-dir build-release --output-on-failure
./build-release/drawer
```

命令行参数：

```bash
./drawer --trigger    # 触发一次抽取
./drawer --edit       # 打开编辑器
./drawer --stats      # 显示统计
./drawer --version    # 显示版本
./drawer --help       # 显示帮助
```