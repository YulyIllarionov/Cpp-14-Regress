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

#include "range_based_for.h"
#include "lambda_function.h"
#include "auto.h"
#include "cpp14_scanner.h"

namespace cpp14regress {

    template<typename VisitorType>
    class Cpp14RegressASTConsumer : public clang::ASTConsumer {
    private:
        VisitorType *visitor;

    public:
        explicit Cpp14RegressASTConsumer(clang::ASTContext *context)
                : visitor(new VisitorType(context)) {}

        virtual void HandleTranslationUnit(clang::ASTContext &context) {
            visitor->TraverseDecl(context.getTranslationUnitDecl());
        }

        virtual void EndFileAction() { visitor->EndFileAction(); }
    };

    template<typename VisitorType>
    class Cpp14RegressFrontendAction : public clang::ASTFrontendAction {
    public:
        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                                      llvm::StringRef file) {
            f_consumer = new Cpp14RegressASTConsumer<VisitorType>(&CI.getASTContext());
            return std::unique_ptr<clang::ASTConsumer>(f_consumer);
        }

        virtual void EndSourceFileAction() { f_consumer->EndFileAction(); }

    private:
        Cpp14RegressASTConsumer<VisitorType> *f_consumer;
    };
}

#endif /*CPP14REGRESS_BASE_TYPES_H*/
