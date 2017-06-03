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
}

#endif /*CPP14REGRESS_TOOL_H*/
