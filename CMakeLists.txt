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
    src/data/data_store.cpp
    src/data/encryptor.cpp
    src/data/legacy_migrator.cpp
    src/platform/single_instance.cpp
    src/platform/auto_launch.cpp
    src/application/application_controller.cpp
    src/ui/tray_manager.cpp
    src/ui/draw_window.cpp
    src/ui/editor_dialog.cpp
    src/ui/password_dialog.cpp
)

set(HEADERS
    src/cli/cli_parser.h
    src/core/name_pool.h
    src/core/random_engine.h
    src/data/config_manager.h
    src/data/data_store.h
    src/data/encryptor.h
    src/data/legacy_migrator.h
    src/platform/single_instance.h
    src/platform/auto_launch.h
    src/application/application_controller.h
    src/ui/tray_manager.h
    src/ui/draw_window.h
    src/ui/editor_dialog.h
    src/ui/password_dialog.h
)

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

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

enable_testing()
add_subdirectory(tests)
