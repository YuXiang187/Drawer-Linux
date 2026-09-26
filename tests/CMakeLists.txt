cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Test)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

set(CMAKE_AUTOMOC ON)

set(TEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)
set(TEST_LIBRARIES Qt6::Core Qt6::Test)

# test_random_engine
add_executable(test_random_engine
    test_random_engine.cpp
    ../src/core/random_engine.cpp
)
target_include_directories(test_random_engine PRIVATE ${TEST_INCLUDE_DIR})
target_compile_options(test_random_engine PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(test_random_engine PRIVATE ${TEST_LIBRARIES})
add_test(NAME RandomEngine COMMAND test_random_engine)

# test_name_pool
add_executable(test_name_pool
    test_name_pool.cpp
    ../src/core/name_pool.cpp
    ../src/core/random_engine.cpp
)
target_include_directories(test_name_pool PRIVATE ${TEST_INCLUDE_DIR})
target_compile_options(test_name_pool PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(test_name_pool PRIVATE ${TEST_LIBRARIES})
add_test(NAME NamePool COMMAND test_name_pool)

# test_config_manager (ConfigManager depends on Encryptor/OpenSSL)
add_executable(test_config_manager
    test_config_manager.cpp
    ../src/data/config_manager.cpp
    ../src/data/encryptor.cpp
)
target_include_directories(test_config_manager PRIVATE ${TEST_INCLUDE_DIR})
target_compile_options(test_config_manager PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(test_config_manager PRIVATE ${TEST_LIBRARIES} OpenSSL::Crypto)
add_test(NAME ConfigManager COMMAND test_config_manager)
