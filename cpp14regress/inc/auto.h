#ifndef CPP14REGRESS_AUTO_H
#define CPP14REGRESS_AUTO_H

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

namespace cpp14regress {

    class AutoReplacer : public FeatureReplacer { //TODO fix
    public:

        AutoReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual cpp14features type() { return cpp14features::auto_keyword; }

        virtual bool VisitTypeLoc(clang::TypeLoc typeLoc);

        virtual bool VisitVarDecl(clang::VarDecl *varDecl);

    };

    /*class AutoReplacer : public clang::RecursiveASTVisitor<AutoReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;

    public:
        explicit AutoReplacer(clang::ASTContext *context, cpp14features_stat *stat,
                              DirectoryGenerator *dg);

        virtual bool VisitTypeLoc(clang::TypeLoc typeLoc);

        virtual void EndFileAction();

        virtual bool VisitVarDecl(clang::VarDecl *declaratorDecl);

        virtual bool VisitFunctionDecl(clang::FunctionDecl *fun);

        //TODO Structured binding declaration
        //TODO template parameter
        //TODO nested-name-specifie

    };*/
}

#endif /*CPP14REGRESS_AUTO_H*/
