#ifndef CPP14REGRESS_USER_LITERALS_H
#define CPP14REGRESS_USER_LITERALS_H

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
#include <vector>
#include <sstream>

namespace cpp14regress {

    class UserLiteralReplacer : public FeatureReplacer {
    private:

        std::string operatorFuncName(std::string s) {
            return std::string("__" + replacement::seed + "_user_literal" + s);
        }

    public:

        UserLiteralReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual features::type type() { return features::type::user_defined_literals; }

        virtual bool VisitFunctionDecl(clang::FunctionDecl *funcDecl);

        virtual bool VisitUserDefinedLiteral(clang::UserDefinedLiteral *literal);
    };

    class StringLiteralMeter : public clang::RecursiveASTVisitor<StringLiteralMeter> {
    private:
        bool f_found = false;
        unsigned f_size = 0;

    public:
        virtual bool VisitStringLiteral(clang::StringLiteral *stringLiteral) {
            f_found = true;
            f_size = stringLiteral->getLength();
            return false;
        }

        bool found() { return f_found; }

        unsigned size() { return f_size; }
    };

}

#endif /*CPP14REGRESS_USER_LITERALS_H*/