set(LLVM_LINK_COMPONENTS support)
include_directories(./inc)

add_clang_library(Cpp14scanner
        ./src/base_types.cpp
        ./src/utils.cpp
        ./src/cpp14_scanner.cpp

        LINK_LIBS
        clangAST
        clangASTMatchers
        clangBasic
        clangDynamicASTMatchers
        clangFrontend
        )

add_clang_library(Cpp14Regress
        ./src/base_types.cpp
        ./src/utils.cpp
        ./src/range_based_for.cpp
        ./src/lambda.cpp
        ./src/auto.cpp
        ./src/decltype.cpp
        ./src/default.cpp
        ./src/constuctor_delegation.cpp
        ./src/raw_string.cpp

        LINK_LIBS
        clangAST
        clangASTMatchers
        clangBasic
        clangDynamicASTMatchers
        clangFrontend
        )

add_subdirectory(tool)
