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
        // @formatter:off
        enum type {
            final_specifier = 0,
            override_specifier,
            explicit_specifier,
            digit_separators,
            binary_literals,
            raw_string_literals,
            member_sizeof,
            alias_type,
            range_based_for,
            delete_keyword,
            default_keyword,
            member_init,
            constuctor_delegation,
            user_defined_literals,
            forward_declared_enum,
            uniform_initialization,
            improved_enum,
            lambda,
            auto_keyword,
            decltype_keyword,

            constexpr_keyword, //found only
            //extern_template,
            init_list, //found only
            trailing_return, //found only
            null_pointer_constant, //found only
            alias_template, //found only
            unrestricted_unions, //found only
            variadic_templates, //found only
            unicode_string_literals, //found only
            //long_long_int,
            noexcept_keyword, //found only
            alignof_operator, //found only
            alignas_specifier, //found only
            //attributes, 
            variable_templates, //found only
            static_assert_decl, //found only
            inline_namespace, //found only
            func_templ_default_args, //found only
            SIZE
        };
        // @formatter:on

        static constexpr unsigned size() { return type::SIZE; }

        static std::string toString(type t);

        static bool isSupported(type t);

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
