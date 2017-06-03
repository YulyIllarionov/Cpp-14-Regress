#ifndef CPP14REGRESS_DIGIT_SEPARATORS_H
#define CPP14REGRESS_DIGIT_SEPARATORS_H

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

    class DigitSeparatorReplacer : public FeatureReplacer { //TODO fix
    public:
        DigitSeparatorReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual features::type type() { return features::type::digit_separators; }

        virtual bool VisitIntegerLiteral(clang::IntegerLiteral *literal);

        virtual bool VisitFloatingLiteral(clang::FloatingLiteral *literal);

    private:
        bool removeSeparators(std::string &literal);
    };


    /*class DigitSeparatorReplacer : public clang::RecursiveASTVisitor<DigitSeparatorReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;

        bool removeSeparators(std::string &literal);

    public:
        explicit DigitSeparatorReplacer(clang::ASTContext *context,
                                        cpp14features_stat *stat, DirectoryGenerator *dg);

        virtual void EndFileAction();

        virtual bool VisitIntegerLiteral(clang::IntegerLiteral *literal);

        virtual bool VisitFloatingLiteral(clang::FloatingLiteral *literal);
    };*/

}

#endif /*CPP14REGRESS_DIGIT_SEPARATORS_H*/
