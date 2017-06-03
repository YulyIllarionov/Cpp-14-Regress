set(LLVM_LINK_COMPONENTS support)
include_directories(./inc)

#add_clang_library(Cpp14scanner
#        ./src/base_types.cpp
#        ./src/utils.cpp
#        ./src/cpp14_scanner.cpp
#
#        LINK_LIBS
#        clangAST
#        clangASTMatchers
#        clangBasic
#        clangDynamicASTMatchers
#        clangFrontend
#        )

add_clang_library(Cpp14Regress
        #./src/ast_to_dot.cpp
        ./src/base_types.cpp
        ./src/utils.cpp
        ./src/tool.cpp
        ./src/cpp14feature.cpp
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
        ./src/strongly_typed_enum.cpp
        ./src/user_literals.cpp
        ./src/final.cpp
        ./src/override.cpp
        ./src/delete.cpp
        ./src/binary_literals.cpp
        ./src/list_init.cpp
        ./src/static_assert.cpp
        ./src/trailing_return.cpp
        ./src/func_templ_default_args.cpp
        ./src/nullptr.cpp
        ./src/alignas.cpp
        ./src/alignof.cpp
        ./src/unicode_literals.cpp
        ./src/inline_namespace.cpp
        ./src/unrestricted_unions.cpp
        ./src/variable_template.cpp
        #./src/file_preparator.cpp

        LINK_LIBS
        clangAST
        clangASTMatchers
        clangBasic
        clangDynamicASTMatchers
        clangFrontend
        )

add_subdirectory(tool)
