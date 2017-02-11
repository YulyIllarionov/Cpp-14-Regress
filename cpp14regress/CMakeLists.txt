set(LLVM_LINK_COMPONENTS support)
include_directories(./inc)
#add_clang_library(RangeBasedForRegress
#        ./src/base_types.cpp
#        ./src/utils.cpp
#        ./src/range_based_for.cpp
#
#        LINK_LIBS
#        clangAST
#        clangASTMatchers
#        clangBasic
#        clangDynamicASTMatchers
#        clangFrontend
#        )

add_clang_library(LambdaFunctionRegress
        ./src/base_types.cpp
        ./src/utils.cpp
        ./src/lambda_function.cpp
        ./src/ast_to_dot.cpp

        LINK_LIBS
        clangAST
        clangASTMatchers
        clangBasic
        clangDynamicASTMatchers
        clangFrontend
        )

add_subdirectory(tool)
