# Drawer-Linux

技术栈：C++20、Qt 6、Qt Widgets、CMake、Linux

运行环境：Debian 13、KDE Plasma 6、Wayland

## 构建

以Debian 13环境为例

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
```

命令行参数：

```bash
./drawer --trigger    # 触发抽取
./drawer --version    # 显示版本
./drawer --help       # 显示帮助
```

## 打包

.deb / .rpm（CPack）打包，Debian 13环境

安装规则：可执行文件安装到 `/usr/bin/drawer`，桌面文件安装到 `/usr/share/applications/drawer.desktop`

生成 rpm 需要 `rpmbuild`：

```bash
sudo apt install rpm
```

构建软件包：

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build-release -j$(nproc)

cd build-release
cpack -G DEB          # drawer_1.0.0_amd64.deb
cpack -G RPM          # drawer-1.0.0-1.x86_64.rpm
```

安装与卸载：

```bash
sudo apt install ./drawer_1.0.0_amd64.deb   # Debian/Ubuntu
sudo dpkg -r drawer

sudo dnf install ./drawer-1.0.0-1.x86_64.rpm # Fedora/openSUSE
sudo rpm -e drawer
```

运行时依赖由打包工具自动推导：deb 使用 `dpkg-shlibdeps`（libqt6core6t64、libqt6gui6、libqt6network6、libqt6widgets6、libssl3t64 等），rpm 使用 `rpmbuild` 的自动依赖（libQt6Core.so.6、libcrypto.so.3 等）

## GNOME桌面运行

GNOME桌面依赖`gnome-shell-extension-appindicator`运行

安装之后启用拓展：`gnome-extensions enable appindicatorsupport@rgcjonas.gmail.com`

检查是否启用：`gnome-extensions list --enabled`

## 卸载后清理

清理残留配置文件：

```bash
rm -r ~/.config/YuXiang/
rm -r ~/.config/autostart/drawer.desktop
```