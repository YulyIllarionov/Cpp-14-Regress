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

#include <string>

namespace cpp14regress {

    class ConstructorDelegationReplacer : public clang::RecursiveASTVisitor<ConstructorDelegationReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        const std::string f_init_name = "Init";

    public:
        explicit ConstructorDelegationReplacer(clang::ASTContext *context, cpp14features_stat *stat);

        virtual void EndFileAction() { f_rewriter->overwriteChangedFiles(); }

        virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl);
    };

    class initFunNameGenerator {
    public:
        initFunNameGenerator (std::string name) { f_name = name + "Init"; }
        inline std::string toString() {
            return f_name;
        }
    private:
        std::string f_name;
    };
}

#endif /*CPP14REGRESS_CONSTRUCTOR_DELEGATION_H*/
