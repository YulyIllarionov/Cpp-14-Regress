add_clang_executable(cpp14regress main.cpp)
include_directories(../inc)
target_link_libraries(cpp14regress
        clangAST
        clangASTMatchers
        clangBasic
        clangDynamicASTMatchers
        clangFrontend
        clangTooling
        Cpp14scanner
        Cpp14Regress
        #RangeBasedForRegress
        #LambdaFunctionRegress
        #AutoRegress
        #DefaultKeywordRegress
        )

