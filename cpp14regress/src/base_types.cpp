#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/AST/ParentMap.h"

#include "base_types.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    string cpp14features_stat::toString(cpp14features f) {
        static const char *features[size()] = {
                "auto keyword",
                "decltype keyword",
                "constexpr keyword",
                "extern template",
                "default specifier",
                "delete specifier",
                "override specifier",
                "final specifier",
                "explicit specifier",
                "initializer list",
                "uniform initialization",
                "range based for",
                "lambda function",
                "alternative function syntax",
                "constuctor delegation",
                "null pointer constant",
                "enum class",
                "right angle bracket",
                "alias template",
                "alias_type",
                "unrestricted unions",
                "variadic templates",
                "raw_string_literals",
                "unicode string literals",
                "user defined literals",
                "long long int",
                "implict sizeof",
                "noexcept keyword",
                "alignof operator",
                "alignas specifier",
                "attributes",
                "variable templates",
                "digit separators"
        };
        return string(features[(size_t) f - (size_t) cpp14features::begin]);
    }

}