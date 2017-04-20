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

#include "base_types.h"
#include "utils.h"

#include <string>

namespace cpp14regress {

    class LambaClassNameGenerator {
    private:
        static const std::string f_name;
        static int f_count;
    public:
        LambaClassNameGenerator() {}

        static std::string toString();

        static std::string generate();
    };

    class LambaClassFieldNameGenerator : public StringGenerator {
    private:
        clang::ValueDecl *f_variable;
    public:
        LambaClassFieldNameGenerator(clang::ValueDecl *variable_) : f_variable(variable_) {}

        virtual std::string toString();
    };

    class GenericTypeGenerator : public StringGenerator {
    private:
        int f_count;
    public:
        GenericTypeGenerator() : f_count(-1) {}

        virtual std::string toString();

        virtual std::string generate();
    };

    class LambdaFunctionReplacer : public clang::RecursiveASTVisitor<LambdaFunctionReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        const std::string f_header_path = "/home/yury/llvm-clang/test/cpp14regress_lambda.h";
        std::string f_code;

    public:
        explicit LambdaFunctionReplacer(clang::ASTContext *context, cpp14features_stat *stat);

        virtual void EndFileAction() {}

        virtual bool VisitLambdaExpr(clang::LambdaExpr *lambda);
    };
}

#endif //CPP14REGRESS_LAMBDA_FUNCTION_H
