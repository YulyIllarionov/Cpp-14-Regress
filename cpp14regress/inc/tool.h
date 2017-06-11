#ifndef CPP14REGRESS_TOOL_H
#define CPP14REGRESS_TOOL_H

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
#include "utils.h"
#include "cpp14feature.h"

namespace cpp14regress {

    class Cpp14RegressASTConsumer : public clang::ASTConsumer {
    public:
        Cpp14RegressASTConsumer(features::type feature, clang::CompilerInstance *ci);

        virtual void HandleTranslationUnit(clang::ASTContext &context) {
            f_visitor->TraverseDecl(context.getTranslationUnitDecl());
        }

        virtual void EndSourceFileAction() { f_visitor->EndSourceFileAction(); }

        virtual void BeginSourceFileAction() { f_visitor->BeginSourceFileAction(); }

        virtual ~Cpp14RegressASTConsumer() { delete f_visitor; }

    private:
        FeatureVisitor *f_visitor = nullptr;
    };

    class Cpp14RegressFrontendAction : public clang::ASTFrontendAction {
    public:

        Cpp14RegressFrontendAction(features::type feature) : f_feature(feature) {}

        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                                      llvm::StringRef file) {
            f_astConsumer = new Cpp14RegressASTConsumer(f_feature, &CI);
            return std::unique_ptr<clang::ASTConsumer>(f_astConsumer);
        }

        virtual void EndSourceFileAction() { f_astConsumer->EndSourceFileAction(); }

        //TODO segmentation fault
        //virtual bool BeginSourceFileAction(clang::CompilerInstance &CI, llvm::StringRef Filename) {
        //    f_consumer->BeginSourceFileAction();
        //    return true;
        //}

        //virtual ~Cpp14RegressFrontendAction() { delete f_astConsumer; }

    private:
        Cpp14RegressASTConsumer *f_astConsumer;
        features::type f_feature;
    };

    class Cpp14RegressFrontendActionFactory : public clang::tooling::FrontendActionFactory {
    public:
        Cpp14RegressFrontendActionFactory(features::type feature) : f_feature(feature) {}

        clang::FrontendAction *create() {
            return new Cpp14RegressFrontendAction(f_feature);
        }

    private:
        features::type f_feature;
    };

}

#endif /*CPP14REGRESS_TOOL_H*/
