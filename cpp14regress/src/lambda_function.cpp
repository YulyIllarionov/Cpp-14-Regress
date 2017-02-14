
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

#include "lambda_function.h"
#include "utils.h"
#include "ast_to_dot.h"

#include <iostream>
#include <string>

namespace cpp14regress {

    using namespace std;
    using namespace clang;

    const string LambaClassNameGenerator::f_name = "__cpp14regress_lambda_";
    int LambaClassNameGenerator::f_count = -1;

    string LambaClassNameGenerator::toString() {
        return string(f_name + to_string(f_count));
    }

    string LambaClassNameGenerator::generate() {
        f_count++;
        return toString();
    }

    string LambaClassFieldNameGenerator::toString() {
        return string("f_" + f_variable->getNameAsString());
    }

    LambdaFunctionReplacer::LambdaFunctionReplacer(ASTContext *context) {
        f_context = context;
        f_rewriter = new Rewriter(context->getSourceManager(),
                                  context->getLangOpts());
    }

    //TODO generic?
    //TODO incapture initialization
    bool LambdaFunctionReplacer::VisitLambdaExpr(LambdaExpr *lambda) {
        static int count = 0;
        //ast_graph ag(lambda, f_context);
        string dot_filename = "/home/yury/llvm-clang/test/dot/";
        //ag.to_dot_file(dot_filename + "lambda" + to_string(count) + ".dot");

        cout << "Lambda " << count << ": " << endl << "----------------" << endl;

        Indent indent;

        CXXRecordDecl* lambdaClass = lambda->getLambdaClass();

        for (auto it = lambdaClass->ctor_begin(); it != lambdaClass->ctor_end(); it++) {
            //static int j = 0;
            //ast_graph ag(it->getBody(), f_context);
            //ag.to_dot_file(dot_filename + "lambda" + to_string(j) + ".dot");
            //j++;
            for (size_t i = 0; i != it->param_size(); i++) {
                ParmVarDecl* parameter = it->getParamDecl(i);
                cout << parameter->getType().getAsString() << endl
                     << parameter->getQualifiedNameAsString() << endl;
            }
            cout << "---" << endl;
        }
        cout << "class " << LambaClassNameGenerator::generate() << " {" << endl;
        cout << "private:" << endl;
        //Lambda class fields
        for (auto it = lambda->captures().begin(); it != lambda->captures().end(); it++) {
            VarDecl *captured_var = it->getCapturedVar();
            cout << captured_var->getType().getAsString() << " "
                 << captured_var->getNameAsString() << endl; //TODO add f_

        }
        cout << "public:" << endl;
        //Lambda class constructor
        cout << LambaClassNameGenerator::toString() << "(";
        for (auto it = lambda->capture_begin(); it != lambda->capture_end();) {
            VarDecl *captured_var = it->getCapturedVar();
            cout << captured_var->getType().getAsString() << " "
                 << captured_var->getNameAsString() << "_"
                 << ((++it != lambda->capture_end()) ? ", " : ") ");
        }
        cout << "{" << endl;
        //Lambda class constructor body
        for (auto it = lambda->capture_begin(); it != lambda->capture_end(); it++) {
            VarDecl *captured_var = it->getCapturedVar();
            cout << captured_var->getNameAsString() //TODO add f_
                 << " = " << captured_var->getNameAsString() << "_" << ";" << endl;
        }
        cout << "}" << endl;
        //Lambda class operator()
        FunctionDecl* lambdaFunction = lambda->getCallOperator();
        cout << QualType::getAsString(lambdaFunction->getReturnType().getSplitDesugaredType())//XXX
             << " operator ()(";
        for (size_t i = 0; i != lambdaFunction->param_size();) {
            ParmVarDecl* parameter = lambdaFunction->getParamDecl(i);
            cout << parameter->getType().getAsString() << " "
                 << parameter->getQualifiedNameAsString()
                 << ((++i != lambdaFunction->param_size()) ? ", " : ") ");
        }
        cout << stringFromStmt(lambda->getBody(), f_context) << endl;
        cout << "};" << endl;
        cout << "----------------" << endl << endl;
        count++;
        return true;
    }

}