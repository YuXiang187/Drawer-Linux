cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Test Network)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

set(COMMON_SOURCES
    ../src/core/name_pool.cpp
    ../src/core/random_engine.cpp
    ../src/data/config_manager.cpp
    ../src/data/encryptor.cpp
    ../src/data/legacy_migrator.cpp
    ../src/platform/single_instance.cpp
    ../src/platform/auto_launch.cpp
)

set(COMMON_LINKS Qt6::Core Qt6::Test Qt6::Network OpenSSL::Crypto)

# test_name_pool
add_executable(test_name_pool test_name_pool.cpp ${COMMON_SOURCES})
target_include_directories(test_name_pool PRIVATE ../src)
target_link_libraries(test_name_pool PRIVATE ${COMMON_LINKS})
add_test(NAME NamePool COMMAND test_name_pool)

# test_config_manager
add_executable(test_config_manager test_config_manager.cpp ${COMMON_SOURCES})
target_include_directories(test_config_manager PRIVATE ../src)
target_link_libraries(test_config_manager PRIVATE ${COMMON_LINKS})
add_test(NAME ConfigManager COMMAND test_config_manager)

# test_random_engine
add_executable(test_random_engine test_random_engine.cpp ${COMMON_SOURCES})
target_include_directories(test_random_engine PRIVATE ../src)
target_link_libraries(test_random_engine PRIVATE ${COMMON_LINKS})
add_test(NAME RandomEngine COMMAND test_random_engine)

# test_encryptor
add_executable(test_encryptor test_encryptor.cpp ${COMMON_SOURCES})
target_include_directories(test_encryptor PRIVATE ../src)
target_link_libraries(test_encryptor PRIVATE ${COMMON_LINKS})
add_test(NAME Encryptor COMMAND test_encryptor)

# test_legacy_migrator
add_executable(test_legacy_migrator test_legacy_migrator.cpp ${COMMON_SOURCES})
target_include_directories(test_legacy_migrator PRIVATE ../src)
target_link_libraries(test_legacy_migrator PRIVATE ${COMMON_LINKS})
add_test(NAME LegacyMigrator COMMAND test_legacy_migrator)

# test_single_instance
add_executable(test_single_instance test_single_instance.cpp ${COMMON_SOURCES})
target_include_directories(test_single_instance PRIVATE ../src)
target_link_libraries(test_single_instance PRIVATE ${COMMON_LINKS})
add_test(NAME SingleInstance COMMAND test_single_instance)

# test_auto_launch
add_executable(test_auto_launch test_auto_launch.cpp ${COMMON_SOURCES})
target_include_directories(test_auto_launch PRIVATE ../src)
target_link_libraries(test_auto_launch PRIVATE ${COMMON_LINKS})
add_test(NAME AutoLaunch COMMAND test_auto_launch)
