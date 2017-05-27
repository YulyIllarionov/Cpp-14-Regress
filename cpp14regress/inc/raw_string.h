#ifndef CPP14REGRESS_RAW_STRING_H
#define CPP14REGRESS_RAW_STRING_H

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

    class RawStringReplacer : public FeatureReplacer { //TODO fix
    public:
        RawStringReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual cpp14features type() { return cpp14features::raw_string_literals; }

        virtual bool VisitStringLiteral(clang::StringLiteral *literal);

    private:
        inline bool escapeCharToString(char c, std::string &s);
    };

    /*class RawStringReplacer : public clang::RecursiveASTVisitor<RawStringReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;



    public:
        explicit RawStringReplacer(clang::ASTContext *context,
                                   cpp14features_stat *stat, DirectoryGenerator *dg);

        virtual void EndFileAction();

        virtual bool VisitStringLiteral(clang::StringLiteral *literal);
    };*/

}

#endif /*CPP14REGRESS_RAW_STRING_H*/
