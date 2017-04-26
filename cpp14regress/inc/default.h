#ifndef CPP14REGRESS_DEFAULT_KEYWORD_H
#define CPP14REGRESS_DEFAULT_KEYWORD_H

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
    class DefaultKeywordReplacer : public clang::RecursiveASTVisitor<DefaultKeywordReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;
public:
        explicit DefaultKeywordReplacer(clang::ASTContext *context, cpp14features_stat *stat, DirectoryGenerator *dg);

        virtual void EndFileAction();

        virtual bool VisitCXXMethodDecl(clang::CXXMethodDecl *methodDecl);
    };
}

#endif //CPP14REGRESS_DEFAULT_KEYWORD_H
