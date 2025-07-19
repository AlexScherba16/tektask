cmake_minimum_required(VERSION 3.31)
project(se_solver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Enable unit tests build" OFF)

add_subdirectory(lib)

if (BUILD_TESTS)
    message(STATUS "Build with unit tests")
    add_subdirectory(test)
else ()
    message(STATUS "Skip unit tests building")
endif ()

add_executable(${CMAKE_PROJECT_NAME} app/main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE se_solver_lib)
