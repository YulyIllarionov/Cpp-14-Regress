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
                "auto keyword", //found
                "decltype keyword", //found
                "constexpr keyword", //found
                "extern template", //?
                "default specifier", //found
                "delete specifier", //found
                "override specifier", //?
                "final specifier", //?
                "explicit specifier", //found
                "initializer list", //?
                "uniform initialization", //?
                "range based for", //found
                "lambda function", //found
                "alternative function syntax", //?
                "constuctor delegation", //found
                "null pointer constant", //?
                "enum class", //found
                "right angle bracket", //?
                "typedef template", //?
                "unrestricted unions", //?
                "variadic templates", //?
                "unicode string literals", //found
                "user defined literals", //?
                "long long int", //?
                "implict sizeof", //?
                "noexcept keyword", //found
                "alignof operator", //?
                "alignas operator", //?
                "attributes", //?
                "variable templates", //?
                "digit separators" //found
        };
        return string(features[(size_t) f - (size_t) cpp14features::begin]);
    }

}