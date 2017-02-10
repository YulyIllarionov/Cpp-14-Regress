#cmake file for clion ide

cmake_minimum_required(VERSION 3.6)
project(cpp14regress_ide)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES ./cpp14regress/tool/main.cpp )

include_directories(./../../../include/ ./cpp14regress/inc/)

set(HEADER_FILES ./cpp14regress/inc/base_types.h
        ./cpp14regress/inc/utils.h
        ./cpp14regress/inc/range_based_for.h
        ./cpp14regress/inc/lambda_function.h
        ./cpp14regress/inc/ast_to_dot.h)

set(SOURCE_FILES ./cpp14regress/src/base_types.cpp
        ./cpp14regress/src/utils.cpp
        ./cpp14regress/src/range_based_for.cpp
        ./cpp14regress/src/lambda_function.cpp
        ./cpp14regress/src/ast_to_dot.cpp)

add_executable(cpp14regress_ide ${SOURCE_FILES})
