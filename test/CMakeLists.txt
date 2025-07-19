project(se_solver_test)

find_package(GTest REQUIRED)

enable_testing()

add_executable(se_solver_test unit/test_main.cpp
        unit/cli_test/cli_test.cpp
        unit/queue_test/blocking_queue_test.cpp
        unit/resolver_test/quadratic_resolver_test.cpp
)

target_include_directories(se_solver_test PRIVATE ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/test)
target_link_libraries(se_solver_test PRIVATE se_solver_lib GTest::gtest GTest::gtest_main GTest::gmock)
