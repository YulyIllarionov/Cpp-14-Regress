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

    class RangeBasedForASTConsumer : public clang::ASTConsumer {
    private:
        Cpp14scanner *visitor;

    public:
        explicit RangeBasedForASTConsumer(clang::ASTContext *context)
                : visitor(new Cpp14scanner(context)) {}

        virtual void HandleTranslationUnit(clang::ASTContext &context);
    };

    class RangeBasedForFrontendAction : public clang::ASTFrontendAction {
    public:
        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                                      llvm::StringRef file);
    };

}

#endif /*CPP14REGRESS_BASE_TYPES_H*/
