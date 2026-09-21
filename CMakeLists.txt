cmake_minimum_required(VERSION 3.16)
project(Drawer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

set(SOURCES
    src/main.cpp
    src/cli/cli_parser.cpp
    src/core/name_pool.cpp
    src/core/random_engine.cpp
    src/data/config_manager.cpp
    src/data/encryptor.cpp
    src/platform/single_instance.cpp
    src/platform/auto_launch.cpp
    src/application/application_controller.cpp
    src/ui/tray_manager.cpp
    src/ui/draw_window.cpp
    src/ui/floating_window.cpp
    src/ui/editor_dialog.cpp
    src/ui/password_dialog.cpp
)

set(HEADERS
    src/cli/cli_parser.h
    src/core/name_pool.h
    src/core/random_engine.h
    src/data/config_manager.h
    src/data/encryptor.h
    src/platform/single_instance.h
    src/platform/auto_launch.h
    src/application/application_controller.h
    src/ui/tray_manager.h
    src/ui/draw_window.h
    src/ui/floating_window.h
    src/ui/editor_dialog.h
    src/ui/password_dialog.h
)

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    OpenSSL::Crypto
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME drawer
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

qt_finalize_executable(${PROJECT_NAME})

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(UNIX AND NOT APPLE)
    # Desktop entry -> /usr/share/applications/drawer.desktop
    install(FILES packaging/drawer.desktop
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
    )
endif()

# ---------------------------------------------------------------------------
# CPack packaging (.deb / .rpm)
# ---------------------------------------------------------------------------
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB;RPM")

    set(CPACK_PACKAGE_NAME "drawer")
    set(CPACK_PACKAGE_VENDOR "YuXiang")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Name Random Drawer")
    string(CONCAT CPACK_PACKAGE_DESCRIPTION
        "YuXiang Drawer is a small tray application that draws a random name "
        "from a user managed name pool. The pool is stored encrypted and can "
        "be maintained from the tray icon menu.")
    set(CPACK_PACKAGE_CONTACT "YuXiang187")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
    set(CPACK_PACKAGE_EXECUTABLES "drawer" "YuXiang Drawer")
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_VERBATIM_VARIABLES TRUE)

    # Packages always install below /usr, independent of CMAKE_INSTALL_PREFIX
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

    # --- .deb ---------------------------------------------------------------
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    # Derive Depends: from the ELF objects (libqt6*, libssl, libc, ...)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

    # --- .rpm ---------------------------------------------------------------
    set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
    set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Utility")
    set(CPACK_RPM_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}")

    include(CPack)
endif()

enable_testing()
add_subdirectory(tests)
