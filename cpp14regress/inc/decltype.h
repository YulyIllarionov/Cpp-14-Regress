#ifndef CPP14REGRESS_DECLTYPE_H
#define CPP14REGRESS_DECLTYPE_H

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

    class DecltypeReplacer : public FeatureReplacer { //TODO fix
    public:

        DecltypeReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual cpp14features type() { return cpp14features::decltype_keyword; }

        //virtual bool VisitTypeLoc(clang::TypeLoc typeLoc);

        virtual bool VisitDecltypeTypeLoc(clang::DecltypeTypeLoc typeLoc);

        //virtual bool VisitDeclaratorDecl(clang::DeclaratorDecl *valueDecl);
    };

    /*class DecltypeReplacer : public clang::RecursiveASTVisitor<DecltypeReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;

    public:
        explicit DecltypeReplacer(clang::ASTContext *context, cpp14features_stat *stat, DirectoryGenerator *dg);

        virtual void EndFileAction(); //TODO add check
        virtual bool VisitDeclaratorDecl(clang::DeclaratorDecl *valueDecl);
    };*/

}

#endif /*CPP14REGRESS_DECLTYPE_H*/
