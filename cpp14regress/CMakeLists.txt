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
        ./src/ast_to_dot.cpp
        ./src/base_types.cpp
        ./src/utils.cpp
        ./src/range_based_for.cpp
        ./src/lambda.cpp
        ./src/auto.cpp
        ./src/decltype.cpp
        ./src/default.cpp
        ./src/constuctor_delegation.cpp
        ./src/raw_string.cpp
        ./src/implict_sizeof.cpp
        ./src/digit_separators.cpp
        ./src/alias_type.cpp
        ./src/uniform_initialization.cpp
        ./src/explicit_conversion.cpp
        ./src/in_class_init.cpp

        LINK_LIBS
        clangAST
        clangASTMatchers
        clangBasic
        clangDynamicASTMatchers
        clangFrontend
        )

add_subdirectory(tool)
