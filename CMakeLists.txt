
cmake_minimum_required(VERSION 3.16)

project(GeneralStoreManagementSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required libraries
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(GTest REQUIRED)

# Application executable
add_executable(store
    main.cpp
    src/Database.cpp
    src/Authentication.cpp
    src/Product.cpp
    src/Customer.cpp
)

target_include_directories(store PRIVATE include)

target_link_libraries(store
    PRIVATE
    SQLite::SQLite3
    OpenSSL::Crypto
)

# Google Test executable
enable_testing()

add_executable(store_tests
    tests/DatabaseTest.cpp
    src/Database.cpp
    src/Authentication.cpp
    src/Product.cpp
    src/Customer.cpp
)

target_include_directories(store_tests PRIVATE include)

target_link_libraries(store_tests
    PRIVATE
    SQLite::SQLite3
    OpenSSL::Crypto
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(store_tests)

# Enable code coverage
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(store_tests PRIVATE
        --coverage
        -O0
        -g
    )

    target_link_options(store_tests PRIVATE
        --coverage
    )
endif()

# Run Google Tests
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS store_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Generate HTML coverage report
find_program(GCOVR_EXECUTABLE gcovr)

if(GCOVR_EXECUTABLE)
    add_custom_target(coverage
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${GCOVR_EXECUTABLE}
            --root ${CMAKE_SOURCE_DIR}
            --object-directory ${CMAKE_BINARY_DIR}
            --filter "${CMAKE_SOURCE_DIR}/src/.*"
            --html-details ${CMAKE_BINARY_DIR}/coverage.html
        DEPENDS store_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Clean old reports and test database
add_custom_target(clean_reports
    COMMAND ${CMAKE_COMMAND} -E rm -f
        ${CMAKE_BINARY_DIR}/coverage.html
        ${CMAKE_BINARY_DIR}/coverage.css
        ${CMAKE_BINARY_DIR}/coverage.functions.html
        ${CMAKE_SOURCE_DIR}/coverage.html
        ${CMAKE_SOURCE_DIR}/coverage.css
        ${CMAKE_SOURCE_DIR}/coverage.functions.html
        ${CMAKE_SOURCE_DIR}/database/test_store.db
    COMMAND ${CMAKE_COMMAND} -E rm -f
        ${CMAKE_BINARY_DIR}/*.gcda
        ${CMAKE_BINARY_DIR}/*.gcno
    COMMENT "Cleaning old reports and test data..."
)
