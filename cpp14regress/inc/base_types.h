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


namespace cpp14regress {

    enum class cpp14features {
        begin,
        auto_keyword = begin, //found
        decltype_keyword, //found
        constexpr_keyword, //found
        extern_template, //?
        default_specifier, //found
        delete_specifier, //found
        override_specifier, //found
        final_specifier, //found
        explicit_specifier, //found
        initializer_list, //?
        uniform_initialization, //?
        range_based_for, //found
        lambda_function, //found
        alternative_function_syntax, //?
        constuctor_delegation, //found
        null_pointer_constant, //found
        enum_class, //found
        right_angle_bracket, //?
        alias_template, //found
        alias_type, //found
        unrestricted_unions, //? //boost::variant или placement-new
        variadic_templates, //?
        raw_string_literals, //?
        unicode_string_literals, //found
        user_defined_literals, //?
        long_long_int, //?
        implict_sizeof, //found //TODO sizeof... ??
        noexcept_keyword, //found
        alignof_operator, //?
        alignas_operator, //?
        attributes, //?
        variable_templates, //?
        digit_separators, //found
        end
    };

    class cpp14features_stat {
    private:
        static unsigned const f_size = (int) cpp14features::end - (int) cpp14features::begin;
        std::vector<clang::SourceLocation> f_features[f_size];

    public:
        cpp14features_stat() {}

        static constexpr unsigned size() { return (f_size); }

        size_t size(cpp14features i) { return f_features[(int) i].size(); }

        std::vector<clang::SourceLocation> &operator[](cpp14features f) { return f_features[(int) f]; }

        void push(cpp14features f, clang::SourceLocation sl) { f_features[(int) f].push_back(sl); }

        static std::string toString(cpp14features f);
    };

    template<typename VisitorType>
    class Cpp14RegressASTConsumer : public clang::ASTConsumer {
    public:
        explicit Cpp14RegressASTConsumer(clang::ASTContext *context, cpp14features_stat *stat)
                : visitor(new VisitorType(context, stat)) {}

        virtual void HandleTranslationUnit(clang::ASTContext &context) {
            visitor->TraverseDecl(context.getTranslationUnitDecl());
        }

        virtual void EndFileAction() { visitor->EndFileAction(); }

    private:
        VisitorType *visitor;
    };

    template<typename VisitorType>
    class Cpp14RegressFrontendAction : public clang::ASTFrontendAction {
    public:
        Cpp14RegressFrontendAction(cpp14features_stat *stat) : f_stat(stat) {}

        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                                      llvm::StringRef file) {
            f_consumer = new Cpp14RegressASTConsumer<VisitorType>(&CI.getASTContext(), f_stat);
            return std::unique_ptr<clang::ASTConsumer>(f_consumer);
        }

        virtual void EndSourceFileAction() { f_consumer->EndFileAction(); }

    private:
        Cpp14RegressASTConsumer<VisitorType> *f_consumer;
        cpp14features_stat *f_stat;
    };

    template<typename VisitorType>
    class Cpp14RegressFrontendActionFactory : public clang::tooling::FrontendActionFactory {
    public:
        Cpp14RegressFrontendActionFactory(cpp14features_stat *stat) : f_stat(stat) {}

        clang::FrontendAction *create() { return new Cpp14RegressFrontendAction<VisitorType>(f_stat); }

    private:
        cpp14features_stat *f_stat;
    };

    template<typename VisitorType>
    struct Cpp14scannerConsumerCreator {
        std::unique_ptr<clang::ASTConsumer> newASTConsumer() {
            clang::ASTConsumer *consumer = new Cpp14RegressASTConsumer<VisitorType>();
            return std::unique_ptr<clang::ASTConsumer>(consumer);
        }
    };
}

#endif /*CPP14REGRESS_BASE_TYPES_H*/
