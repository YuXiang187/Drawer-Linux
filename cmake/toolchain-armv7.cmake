# Cross build for 32-bit ARMv7 hard-float (armhf / armv7hl) Debian and Ubuntu
# targets, e.g. Raspberry Pi OS 32-bit.
#
#   sudo dpkg --add-architecture armhf
#   sudo apt update
#   sudo apt install crossbuild-essential-armhf qt6-base-dev:armhf libssl-dev:armhf
#
#   cmake -S . -B build-armhf -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
#       -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-armv7.cmake -DQT_HOST_PATH=/usr
#
# Ubuntu serves the foreign architecture from ports.ubuntu.com, which needs an
# extra entry in sources.list.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7l)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# Target libraries live in the multiarch directory of the tuple
set(CMAKE_LIBRARY_ARCHITECTURE arm-linux-gnueabihf)
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/arm-linux-gnueabihf/cmake")

# Build tools (moc, uic, rcc, ...) keep coming from the host
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# pkg-config must not report the libraries of the host architecture
set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/share/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "/")
