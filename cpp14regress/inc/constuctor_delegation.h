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

#include "cpp14feature.h"

#include <iostream>
#include <string>

namespace cpp14regress {

    class ConstructorDelegationReplacer : public FeatureReplacer {
    public:

        ConstructorDelegationReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual features::type type() { return features::type::constuctor_delegation; }

        virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl);

    private:
        const std::string f_seed = "Init_cpp14regress";

        std::string initFunName(const clang::CXXConstructorDecl *ctr);

        std::string initFunCall(const clang::CXXConstructorDecl *delegating);
    };

    class ParamSearcher : public clang::RecursiveASTVisitor<ParamSearcher> {
    private:
        typedef clang::FunctionDecl::param_const_iterator param_iterator;

        param_iterator f_begin;
        param_iterator f_end;
        bool f_found = false;
        unsigned f_index = 0;

    public:
        ParamSearcher(param_iterator begin, param_iterator end) :
                f_begin(begin), f_end(end) {}

        virtual bool VisitDeclRefExpr(clang::DeclRefExpr *dre);

        bool found() { return f_found; }

        unsigned index() { return f_index; }
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
