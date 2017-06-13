#ifndef CPP14REGRESS_LAMBDA_FUNCTION_H
#define CPP14REGRESS_LAMBDA_FUNCTION_H

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
#include "utils.h"

#include <string>
#include <sstream>
#include <algorithm>

namespace cpp14regress {

    /*class VariableToField : public StringGenerator {
    private:
        const clang::ValueDecl *f_variable;
    public:
        VariableToField(const clang::ValueDecl *variable) : f_variable(variable) {}

        virtual std::string toString();
    };*/

    struct LambdaClassName {
        static std::string toString(unsigned i);
    };

    struct LambdaHeaderName {
        static std::string toString(unsigned i, std::string seed);
    };

    struct LambdaHeaderGuard {
        static std::string toString(unsigned i, std::string seed);
    };

    struct GenericType {
        static std::string toString(unsigned i);
    };

    class IncludeLocSearcher : public InParentSearcher {
    private:
        clang::SourceLocation f_location;

        virtual bool checkDecl(const clang::Decl *decl);

    public:
        IncludeLocSearcher(clang::ASTContext *astContext) : InParentSearcher(astContext) {}

        clang::SourceLocation find(const clang::Stmt *stmt);
    };

    class InParentTemplateSearcher : public InParentSearcher {
    private:
        const clang::RedeclarableTemplateDecl *f_templateDecl = nullptr;

        virtual bool checkDecl(const clang::Decl *decl);

    public:
        InParentTemplateSearcher(clang::ASTContext *astContext) : InParentSearcher(astContext) {}

        const clang::RedeclarableTemplateDecl *find(const clang::Stmt *stmt);
    };


    class LambdaReplacer : public FeatureVisitor {
    public:
        typedef NameGenerator<LambdaClassName> LambdaClassNameGenerator;
        typedef NameGenerator<LambdaHeaderName> LambdaHeaderNameGenerator;
        typedef NameGenerator<LambdaHeaderGuard> LambdaHeaderGuardGenerator;
        typedef NameGenerator<GenericType> GenericTypeGenerator;

        LambdaReplacer(clang::CompilerInstance *ci) : FeatureVisitor(ci) {}

        virtual features::type type() { return features::type::lambda; }

        //virtual bool VisitCallExpr(clang::CallExpr *lambdaCall);

        virtual bool VisitLambdaExpr(clang::LambdaExpr *lambda);
    };
}

#endif //CPP14REGRESS_LAMBDA_FUNCTION_H
