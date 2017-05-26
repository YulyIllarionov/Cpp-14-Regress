#ifndef CPP14REGRESS_UNIFORM_INITIALIZATION_H
#define CPP14REGRESS_UNIFORM_INITIALIZATION_H

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

#include <iostream>
#include <string>

namespace cpp14regress {

    class UniformInitReplacer : public FeatureReplacer {
    public:

        UniformInitReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual cpp14features type() { return cpp14features::uniform_initialization; }

        virtual bool VisitCXXConstructExpr(clang::CXXConstructExpr *constructExpr);
    };

    /*class UniformInitReplacer : public clang::RecursiveASTVisitor<UniformInitReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;
        
    public:
        explicit UniformInitReplacer(clang::ASTContext *context,
                                     cpp14features_stat *stat, DirectoryGenerator *dg);

        virtual void EndFileAction();

        virtual bool VisitVarDecl(clang::VarDecl *varDecl);

        virtual bool VisitCXXConstructExpr(clang::CXXConstructExpr *constructExpr);

        virtual bool VisitCXXScalarValueInitExpr(clang::CXXScalarValueInitExpr *scalarInitExpr);

        virtual bool VisitInitListExpr(clang::InitListExpr *initListExpr);
    };*/


    class StdListInitSearcher : public clang::RecursiveASTVisitor<StdListInitSearcher> {
    private:

        bool f_found = false;

    public:

        StdListInitSearcher() {}

        virtual bool VisitCXXStdInitializerListExpr(clang::CXXStdInitializerListExpr *list) {
            f_found = true;
            return false;
        }

        bool found() { return f_found; }
    };

    class ListInitSearcher : public clang::RecursiveASTVisitor<ListInitSearcher> {
    private:

        bool f_found = false;

    public:

        ListInitSearcher() {}

        virtual bool VisitInitListExpr(clang::InitListExpr *list) {
            f_found = true;
            return false;
        }

        bool found() { return f_found; }
    };
}

#endif /*CPP14REGRESS_UNIFORM_INITIALIZATION_H*/
