#ifndef CPP14REGRESS_RANGE_BASED_FOR_H
#define CPP14REGRESS_RANGE_BASED_FOR_H

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

#include "utils.h"
#include "base_types.h"
#include <string>

namespace cpp14regress {

    class VariableToPointer : public StringGenerator {
    private:
        clang::ValueDecl *f_variable;
    public:
        VariableToPointer(clang::ValueDecl *variable_)
                : f_variable(variable_) {}

        virtual std::string toString();
    };

    class VariableToArrayElement : public StringGenerator {
    private:
        clang::ValueDecl *f_variable;
        clang::ValueDecl *f_array;
    public:
        VariableToArrayElement(clang::ValueDecl *variable, clang::ValueDecl *array) :
                f_variable(variable), f_array(array) {}

        virtual std::string toString();
    };

    class RangeBasedForReplacer : public FeatureReplacer {
    public:

        RangeBasedForReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual cpp14features type() { return cpp14features::range_based_for; }

        virtual bool VisitCXXForRangeStmt(clang::CXXForRangeStmt *rbf);
    };
}

#endif /*CPP14REGRESS_RANGE_BASED_FOR_H*/
