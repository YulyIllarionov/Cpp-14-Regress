#ifndef CPP14REGRESS_BASE_TYPES_H
#define CPP14REGRESS_BASE_TYPES_H

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

#include "utils.h"
#include <iostream>
#include <string>

namespace cpp14regress {

    class Comment {
    public :

        static std::string line(const std::string &text);

        struct line {
            static std::string begin() { return "//"; }
        };

        static std::string block(const std::string &text);

        struct block {
            static std::string begin() { return "/*"; }

            static std::string end() { return "*/"; }
        };
    };

    struct features {
        enum type {
            auto_keyword = 0,    
            decltype_keyword,    
            constexpr_keyword, 
            //extern_template, 
            default_keyword,     
            delete_keyword, 
            override_specifier, 
            final_specifier, 
            explicit_specifier,      
            initializer_list, 
            uniform_initialization,      
            range_based_for,     
            lambda,     
            trailing_return, 
            constuctor_delegation,   
            null_pointer_constant, 
            improved_enum, 
            alias_template, 
            alias_type, 
            unrestricted_unions, 
            variadic_templates, 
            raw_string_literals,
            unicode_string_literals, 
            user_defined_literals,
            //long_long_int, 
            member_sizeof,      
            noexcept_keyword, 
            alignof_operator, 
            alignas_specifier, 
            //attributes, 
            variable_templates, 
            digit_separators,    
            binary_literals,
            member_init,
            static_assert_decl,
            inline_namespace,
            func_templ_default_args,
            SIZE
        };

        static constexpr unsigned size() { return type::SIZE; }

        static std::string toString(type t) {
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
    };

    namespace replacement {
        enum class result {
            found = 0,
            removed,
            replaced,
            unsureReplaced,
            inserted,
            error
        };

        static const std::string resultStrings[]{"found",
                                                 "removed",
                                                 "replaced",
                                                 "unsure replaced",
                                                 "inserted",
                                                 "error"};

        static const std::string seed("cpp14regress");

        std::string info(features::type f, result r);

        std::string begin(features::type f, result r);

        std::string end(features::type f, result r);
    }
}

#endif /*CPP14REGRESS_BASE_TYPES_H*/
