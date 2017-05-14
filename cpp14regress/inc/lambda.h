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
#include <sstream>

namespace cpp14regress {

    class LambdaClassNameGenerator : public NameGenerator {
    public:
        virtual std::string toString() {
            return std::string("__cpp14regressLambda" + std::to_string(f_count));
        }
    };

    class LambdaHeaderNameGenerator : public NameGenerator {
    public:
        virtual std::string toString() {
            return std::string("cpp14regress_lambda_" + std::to_string(f_count) + ".h");
        }
    };

    class LambdaHeaderGuardGenerator : public NameGenerator {
    public:
        virtual std::string toString() {
            return std::string("CPP14REGRESS_LAMBDA_" + std::to_string(f_count) + "_H");
        }
    };

    class GenericTypeGenerator : public NameGenerator {
    public:
        virtual std::string toString() {
            return std::string("type" + std::to_string(f_count));
        }
    };

    class LambdaReplacer : public FeatureReplacer {
    private:
        std::stringstream f_header;
        static LambdaClassNameGenerator f_lcng;
        static LambdaHeaderNameGenerator f_lhng;
        static LambdaHeaderGuardGenerator f_lhgg;

        virtual void endSourceFileAction();

    public:

        LambdaReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual bool VisitLambdaExpr(clang::LambdaExpr *lambda);

    };

    /*class LambdaFunctionReplacer : public clang::RecursiveASTVisitor<LambdaFunctionReplacer> {
    private:
        clang::ASTContext *f_context;
        clang::Rewriter *f_rewriter;
        cpp14features_stat *f_stat;
        DirectoryGenerator *f_dg;
        const std::string f_header_path = "/home/yury/llvm-clang/test/cpp14regress_lambda.h";
        std::string f_code;

    public:
        explicit LambdaFunctionReplacer(clang::ASTContext *context, cpp14features_stat *stat,
                                        DirectoryGenerator *dg);

        virtual void EndFileAction() {}

        virtual bool VisitLambdaExpr(clang::LambdaExpr *lambda);

    };*/
}

#endif //CPP14REGRESS_LAMBDA_FUNCTION_H
