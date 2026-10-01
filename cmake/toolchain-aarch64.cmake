# Cross build for 64-bit ARM (aarch64 / arm64) Debian and Ubuntu targets.
#
#   sudo dpkg --add-architecture arm64
#   sudo apt update
#   sudo apt install crossbuild-essential-arm64 qt6-base-dev:arm64 libssl-dev:arm64
#
#   cmake -S . -B build-arm64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
#       -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-aarch64.cmake -DQT_HOST_PATH=/usr
#
# Ubuntu serves the foreign architecture from ports.ubuntu.com, which needs an
# extra entry in sources.list.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Target libraries live in the multiarch directory of the tuple
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/aarch64-linux-gnu/cmake")

# Build tools (moc, uic, rcc, ...) keep coming from the host
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# pkg-config must not report the libraries of the host architecture
set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "/")
