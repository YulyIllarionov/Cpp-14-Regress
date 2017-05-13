#ifndef CPP14REGRESS_FILE_PREPARATOR_H
#define CPP14REGRESS_FILE_PREPARATOR_H

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

    class FilesPreparator : public clang::RecursiveASTVisitor<FilesPreparator> {
    private:
        clang::ASTContext *f_context;
        const clang::SourceManager &f_sm;
    public:
        FilesPreparator(clang::ASTContext *context, cpp14features_stat *stat, DirectoryGenerator *dg) :
                f_context(context), f_sm(f_context->getSourceManager()) {}

        virtual void EndFileAction();
    };
}

#endif /*CPP14REGRESS_FILE_PREPARATOR_H*/