#ifndef CPP14REGRESS_LIST_INIT_H
#define CPP14REGRESS_LIST_INIT_H

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

    class ListInitSearcher : public FeatureReplacer {
    public:

        ListInitSearcher(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual features::type type() { return features::type::initializer_list; }

        virtual bool VisitCXXStdInitializerListExpr(clang::CXXStdInitializerListExpr *initList);

    };
}

#endif /*CPP14REGRESS_LIST_INIT_H*/