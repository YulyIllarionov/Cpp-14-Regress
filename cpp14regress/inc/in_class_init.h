#ifndef CPP14REGRESS_IN_CLASS_INIT_H
#define CPP14REGRESS_IN_CLASS_INIT_H

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

    class MemberInitReplacer : public FeatureVisitor {
    public:

        MemberInitReplacer(clang::CompilerInstance *ci) : FeatureVisitor(ci) {}

        virtual features::type type() { return features::type::member_init; }

        virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl);

    };

    /*class MemberInitReplacer : public clang::RecursiveASTVisitor<MemberInitReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;
        const std::string f_extension = "_init";

    public:
        explicit MemberInitReplacer(clang::ASTContext *context,
                                    cpp14features_stat *stat, DirectoryGenerator *dg);

        virtual void EndFileAction();

        virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl);
    };*/

}

#endif /*CPP14REGRESS_IN_CLASS_INIT_H*/
