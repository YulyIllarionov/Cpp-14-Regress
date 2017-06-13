#ifndef CPP14REGRESS_CONSTEXPR_H
#define CPP14REGRESS_CONSTEXPR_H

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

namespace cpp14regress {

    class ConstexprSearcher : public FeatureVisitor {
    public:

        ConstexprSearcher(clang::CompilerInstance *ci) : FeatureVisitor(ci) {}

        virtual features::type type() { return features::type::constexpr_keyword; }

        virtual bool VisitFunctionDecl(clang::FunctionDecl *functionDecl);

        virtual bool VisitVarDecl(clang::VarDecl *varDecl);
    };
}

#endif /*CPP14REGRESS_CONSTEXPR_H*/