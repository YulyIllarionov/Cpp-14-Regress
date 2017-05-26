#ifndef CPP14REGRESS_CONSTRUCTOR_DELEGATION_H
#define CPP14REGRESS_CONSTRUCTOR_DELEGATION_H

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

    class ConstructorDelegationReplacer : public FeatureReplacer {
    public:

        ConstructorDelegationReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual cpp14features type() { return cpp14features::constuctor_delegation; }

        virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl);

    private:
        const std::string f_seed = "Init_cpp14regress";

        std::string initFunName(const clang::CXXConstructorDecl *ctr);

        std::string initFunCall(const clang::CXXConstructorDecl *delegating);
    };

    /*class ConstructorDelegationReplacer : public clang::RecursiveASTVisitor<ConstructorDelegationReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;


    public:
        explicit ConstructorDelegationReplacer(clang::ASTContext *context,
                                               cpp14features_stat *stat, DirectoryGenerator *dg);

        virtual void EndFileAction();

        virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl);
    };*/

}

#endif /*CPP14REGRESS_CONSTRUCTOR_DELEGATION_H*/
