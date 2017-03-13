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
#include <iostream>

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

    template<typename T>
    std::string toSting(T *source, clang::ASTContext* context) {
        const clang::SourceManager &sm = context->getSourceManager();
        const clang::LangOptions &lo = context->getLangOpts();
        clang::SourceLocation b(source->getLocStart()), _e(source->getLocEnd());
        clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, lo));
        return std::string(sm.getCharacterData(b),
                           sm.getCharacterData(e) - sm.getCharacterData(b));
    }

    template<typename T>
    inline bool inProcessedFile(T *source, clang::ASTContext *context){
        if (context->getSourceManager().isInSystemHeader(source->getLocStart()))
            return false;
        return true;
    }

    class Indent
    {
    private:
        unsigned int f_level;
        const static unsigned int f_size = 4;
    public:
        Indent() : f_level(0) {}
        Indent(const Indent &other) = default;
        Indent& operator++();
        Indent operator++(int);
        Indent& operator--();
        Indent operator--(int);
        operator std::string() const {return std::string(f_level * f_size, ' ');}
        friend std::ostream& operator<<(std::ostream& stream, const Indent& indent);
    };
}

#endif /*CPP14REGRESS_UTILS_H*/
