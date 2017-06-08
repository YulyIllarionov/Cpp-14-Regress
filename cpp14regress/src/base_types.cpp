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

#include "cpp14feature.h"
#include <string>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    std::string features::toString(features::type t) {
        std::string s;
        // @formatter:off
            switch (t) {
                case alias_template : s = "alias template"; break;
                case alias_type : s = "alias type"; break;
                case alignas_specifier : s = "alignas"; break;
                case alignof_operator : s = "alignof"; break;
                //case attributes : s = "[[attributes]]"; break;
                case auto_keyword : s = "auto type"; break;
                case binary_literals : s = "binary literal"; break;
                case constexpr_keyword : s = "constexpr"; break;
                case constuctor_delegation : s = "constructor delegation "; break;
                case decltype_keyword : s = "decltype type"; break;
                case default_keyword : s = "default method"; break;
                case delete_keyword : s = "delete "; break;
                case digit_separators : s = "digit separators"; break;
                case explicit_specifier : s = "explicit conversion"; break;
                //case extern_template : s = "extern template"; break;
                case final_specifier : s = "final method"; break;
                case func_templ_default_args : s = "function template default args"; break;
                case improved_enum : s = "improved enum"; break;
                case initializer_list : s = "initializer list"; break;
                case inline_namespace : s = "inline namespace"; break;
                case lambda : s = "lambda"; break;
                //case long_long_int : s = "long long int"; break;
                case member_init : s = "member init"; break;
                case member_sizeof : s = "member sizeof"; break;
                case noexcept_keyword : s = "noexcept keyword"; break;
                case null_pointer_constant : s = "nullptr"; break;
                case override_specifier : s = "override method"; break;
                case range_based_for : s = "range-based for"; break;
                case raw_string_literals : s = "raw string literal"; break;
                case static_assert_decl : s = "static assert"; break;
                case trailing_return : s = "trailing return"; break;
                case unicode_string_literals : s = "unicode string literal"; break;
                case uniform_initialization : s = "uniform initialization"; break;
                case unrestricted_unions : s = "unrestricted union"; break;
                case user_defined_literals : s = "user defined literal"; break;
                case variable_templates : s = "variable template"; break;
                case variadic_templates : s = "variadic template"; break;
                case SIZE : break;
            }
        // @formatter:on
        return s;
    }

    bool features::isSupported(features::type t) {
        bool supported = true;
        // @formatter:off
            switch (t) {
                case alias_template : supported  = false; break;
                //case alias_type : supported  = false; break;
                case alignas_specifier : supported  = false; break;
                case alignof_operator : supported  = false; break;
//NOT SUPPORTED // case attributes : supported  = false; break;
                //case auto_keyword : supported  = false; break;
                case binary_literals : supported  = false; break;
                case constexpr_keyword : supported  = false; break;
                //case constuctor_delegation : supported  = false; break;
                //case decltype_keyword : supported  = false; break;
                //case default_keyword : supported  = false; break;
                //case delete_keyword : supported  = false; break;
                //case digit_separators : supported  = false; break;
                //case explicit_specifier : supported  = false; break;
//NOT SUPPORTED //case extern_template : supported  = false; break;
                //case final_specifier : supported  = false; break;
                case func_templ_default_args : supported  = false; break;
                //case improved_enum : supported  = false; break;
                case initializer_list : supported  = false; break;
                case inline_namespace : supported  = false; break;
                //case lambda : supported  = false; break;
//NOT SUPPORTED //case long_long_int : supported  = false; break;
                //case member_init : supported  = false; break;
                //case member_sizeof : supported  = false; break;
                case noexcept_keyword : supported  = false; break;
                case null_pointer_constant : supported  = false; break;
                //case override_specifier : supported  = false; break;
                //case range_based_for : supported  = false; break;
                //case raw_string_literals : supported  = false; break;
                case static_assert_decl : supported  = false; break;
                case trailing_return : supported  = false; break;
                case unicode_string_literals : supported  = false; break;
                //case uniform_initialization : supported  = false; break;
                case unrestricted_unions : supported  = false; break;
                //case user_defined_literals : supported  = false; break;
                case variable_templates : supported  = false; break;
                case variadic_templates : supported  = false; break;
                case SIZE : supported  = false; break;
                default : break;
            }
        // @formatter:on
        return supported;
    }

    namespace replacement {
        string info(features::type f, result r) {
            return string(seed + " " + features::toString(f) + " " + resultStrings[(int) r]);
        }

        string begin(features::type f, result r) { return info(f, r) + " begin"; }

        string end(features::type f, result r) { return info(f, r) + " end"; }
    }

    string Comment::line(const string &text) {
        string commented(text);
        commented.insert(0, "//");
        for (string::size_type i = 0; i < commented.size(); i++) {
            if (commented[i] == '\n') {
                commented.insert(++i, "//");
                i++;
            }
        }
        return commented;
    }

    string Comment::block(const string &text) {
        return string("/*" + text + "*/");
    }


}