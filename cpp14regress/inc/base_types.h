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

    //class Сommentator {
    //public :
//
    //    static std::string lineComment(const std::string &text);
//
    //    static std::string blockComment(const std::string &text);
    //};

    class DirectoryGenerator {

    public:
        DirectoryGenerator(std::string path, std::string extension);

        //bool createDir(std::string extension);
        std::string getFile(std::string file);

    private:
        std::string f_directory;
        std::string f_extension = "_regressed";
    };

    enum class cpp14features {
        begin,
        auto_keyword = begin, //found   cured?
        decltype_keyword, //found   cured?
        constexpr_keyword, //found
        extern_template, //?
        default_keyword, //found    cured
        delete_keyword, //found
        override_specifier, //found
        final_specifier, //found
        explicit_specifier, //found     cured
        initializer_list, //?
        uniform_initialization, //found     cured
        range_based_for, //found    cured
        lambda_function, //found    cured
        alternative_function_syntax, //found
        constuctor_delegation, //found  cured
        null_pointer_constant, //found
        enum_class, //found
        right_angle_bracket, //?
        alias_template, //found
        alias_type, //found     cured
        unrestricted_unions, //found //boost::variant или placement-new
        variadic_templates, //found
        raw_string_literals, //found    cured
        unicode_string_literals, //found
        user_defined_literals, //found
        long_long_int, //found //TODO literals spec like 12345LL
        implict_sizeof, //found     cured
        noexcept_keyword, //found
        alignof_operator, //found
        alignas_specifier, //found
        attributes, //?
        variable_templates, //found
        digit_separators, //found   cured
        binary_literals, //found
        inclass_init, //found   //cured
        end,
        unknown
    };

    class cpp14features_stat {
    private:
        static unsigned const f_size = (int) cpp14features::end - (int) cpp14features::begin;
        std::vector<clang::SourceLocation> f_features[f_size];

    public:
        cpp14features_stat() {}

        static constexpr unsigned size() { return (f_size); }

        size_t size(cpp14features i) { return f_features[(int) i].size(); }

        std::vector<clang::SourceLocation> &
        operator[](cpp14features f) { return f_features[(int) f]; }

        void push(cpp14features f, clang::SourceLocation sl) { f_features[(int) f].push_back(sl); }

        static std::string toString(cpp14features f);
    };


    class FeatureReplacer : public clang::RecursiveASTVisitor<FeatureReplacer> {
    protected:
        clang::CompilerInstance *f_compilerInstance;
        clang::Rewriter *f_rewriter;

        inline clang::CompilerInstance *compilerInstance() { return f_compilerInstance; }

        inline clang::Rewriter *rewriter() { return f_rewriter; }

        inline clang::ASTContext &astContext() { return f_compilerInstance->getASTContext(); }

        inline clang::SourceManager &sourceManager() {
            return f_compilerInstance->getSourceManager();
        }

        inline clang::LangOptions &langOptions() {
            return f_compilerInstance->getLangOpts();
        }

        virtual void endSourceFileAction() {}

        virtual void beginSourceFileAction() {}

    public:
        FeatureReplacer(clang::CompilerInstance *ci);

        virtual void EndSourceFileAction();

        virtual void BeginSourceFileAction();

        virtual cpp14features type() { return cpp14features::unknown; }

        virtual bool VisitVarDecl(clang::VarDecl *declaratorDecl) { return true; }

        virtual bool VisitLambdaExpr(clang::LambdaExpr *lambda) { return true; }

        virtual bool VisitCXXForRangeStmt(clang::CXXForRangeStmt *for_loop) { return true; }

        virtual bool VisitEnumDecl(clang::EnumDecl *enumDecl) { return true; }

        virtual bool VisitTypeLoc(clang::TypeLoc typeLoc) { return true; }
    };

    template<typename VisitorType>
    class FeatureReplacerASTConsumer : public clang::ASTConsumer {
    public:
        FeatureReplacerASTConsumer(clang::CompilerInstance *ci)
                : f_visitor(new VisitorType(ci)) {}

        virtual void HandleTranslationUnit(clang::ASTContext &context) {
            f_visitor->TraverseDecl(context.getTranslationUnitDecl());
        }

        virtual void EndSourceFileAction() { f_visitor->EndSourceFileAction(); }

        virtual void BeginSourceFileAction() { f_visitor->BeginSourceFileAction(); }

    private:
        VisitorType *f_visitor;
    };

    template<typename VisitorType>
    class FeatureReplacerFrontendAction : public clang::ASTFrontendAction {
    public:

        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                                      llvm::StringRef file) {
            f_consumer = new FeatureReplacerASTConsumer<VisitorType>(&CI);
            return std::unique_ptr<clang::ASTConsumer>(f_consumer);
        }

        virtual void EndSourceFileAction() { f_consumer->EndSourceFileAction(); }

        //TODO segmentation fault
        //virtual bool BeginSourceFileAction(clang::CompilerInstance &CI, llvm::StringRef Filename) {
        //    f_consumer->BeginSourceFileAction();
        //    return true;
        //}

    private:
        FeatureReplacerASTConsumer<VisitorType> *f_consumer;
    };


    template<typename VisitorType>
    class Cpp14RegressASTConsumer : public clang::ASTConsumer {
    public:
        explicit Cpp14RegressASTConsumer(clang::ASTContext *context, cpp14features_stat *stat,
                                         DirectoryGenerator *dg)
                : visitor(new VisitorType(context, stat, dg)) {}

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
        Cpp14RegressFrontendAction(cpp14features_stat *stat, DirectoryGenerator *dg) :
                f_stat(stat), f_dg(dg) {}

        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                                      llvm::StringRef file) {
            f_consumer = new Cpp14RegressASTConsumer<VisitorType>(&CI.getASTContext(), f_stat,
                                                                  f_dg);
            return std::unique_ptr<clang::ASTConsumer>(f_consumer);
        }

        virtual void EndSourceFileAction() { f_consumer->EndFileAction(); }

    private:
        Cpp14RegressASTConsumer<VisitorType> *f_consumer;
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;
    };

    template<typename VisitorType>
    class Cpp14RegressFrontendActionFactory : public clang::tooling::FrontendActionFactory {
    public:
        Cpp14RegressFrontendActionFactory(cpp14features_stat *stat, DirectoryGenerator *dg) :
                f_stat(stat), f_dg(dg) {}

        clang::FrontendAction *create() {
            return new Cpp14RegressFrontendAction<VisitorType>(f_stat, f_dg);
        }

    private:
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;
    };

    template<typename VisitorType>
    struct Cpp14scannerConsumerCreator {
        std::unique_ptr<clang::ASTConsumer> newASTConsumer() {
            clang::ASTConsumer *consumer = new Cpp14RegressASTConsumer<VisitorType>();
            return std::unique_ptr<clang::ASTConsumer>(consumer);
        }
    };

    /*class VarReplacer : public FeatureReplacer {
    public:

        VarReplacer(clang::CompilerInstance *ci) :
                FeatureReplacer(ci) {}

        virtual bool VisitVarDecl(clang::VarDecl *declaratorDecl);
    };*/

}

#endif /*CPP14REGRESS_BASE_TYPES_H*/
