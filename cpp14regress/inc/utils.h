#ifndef CPP14REGRESS_UTILS_H
#define CPP14REGRESS_UTILS_H

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

#include <string>

namespace cpp14regress{

    class StringGenerator{
    public:
        virtual std::string toString() = 0;
    };

    class RecursiveVariableReplacer : public clang::RecursiveASTVisitor<RecursiveVariableReplacer>{
    private:
        const clang::ValueDecl* f_variable;
        StringGenerator* f_generator;
        clang::Rewriter* f_rewriter;

    public:
        RecursiveVariableReplacer(clang::ValueDecl *variable_,
                                  StringGenerator *generator_, clang::Rewriter *rewriter_):
                f_variable(variable_), f_generator(generator_), f_rewriter(rewriter_) {}

        virtual bool  VisitDeclRefExpr(clang::DeclRefExpr *dre);
    };

    template<class CoreClass>
    std::string stringFromSource(CoreClass *cc, clang::ASTContext* context);

}

#endif /*CPP14REGRESS_UTILS_H*/
