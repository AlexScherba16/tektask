project(se_solver_lib)

add_library(se_solver_lib STATIC
        utils/types/types.h
        cli/cli_parser.h
        cli/cli_parser.cpp
        queue/blocking_queue.h
        resolver/quadratic_resolver.h
)

target_include_directories(se_solver_lib PRIVATE ${CMAKE_SOURCE_DIR}/lib)
