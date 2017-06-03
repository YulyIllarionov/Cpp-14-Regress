#ifndef CPP14REGRESS_TRAILING_RETURN_H
#define CPP14REGRESS_TRAILING_RETURN_H

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

    class TrailingReturnSearcher : public FeatureReplacer {
    public:

        TrailingReturnSearcher(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual cpp14features type() { return cpp14features::trailing_return; }

        virtual bool VisitFunctionDecl(clang::FunctionDecl *function);

    };
}

#endif /*CPP14REGRESS_TRAILING_RETURN_H*/