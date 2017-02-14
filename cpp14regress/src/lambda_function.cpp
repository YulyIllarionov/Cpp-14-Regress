
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
#include "llvm/ADT/DenseMap.h"

#include "lambda_function.h"
#include "utils.h"
#include "ast_to_dot.h"

#include <iostream>
#include <string>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

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
        DenseMap<const VarDecl*, FieldDecl*> clangCaptures;
        FieldDecl *thisField;
        lambdaClass->getCaptureFields(clangCaptures, thisField);

        cout << "class " << LambaClassNameGenerator::generate() << " {" << endl;
        if (lambda->capture_begin() != lambda->capture_end())
            cout << "private:" << endl;
        //Lambda class fields
        ++indent;
        for (auto it = lambda->captures().begin(); it != lambda->captures().end(); it++) {
            VarDecl *captured_var = it->getCapturedVar();
            cout << indent << ((lambda->isMutable()) ? "mutable " : "")
                 << clangCaptures[captured_var]->getType().getAsString() << " "
                 << captured_var->getNameAsString() << ";" << endl; //TODO add f_
        }
        cout << "public:" << endl;
        //Lambda class constructor
        cout << indent << LambaClassNameGenerator::toString() << "(";
        for (auto it = lambda->capture_begin(); it != lambda->capture_end();) {
            VarDecl *captured_var = it->getCapturedVar();
            cout << clangCaptures[captured_var]->getType().getAsString() << " "
                 << captured_var->getNameAsString() << "_"
                 << ((++it != lambda->capture_end()) ? ", " : ") : ");
        }
        //Lambda class constructor body
        for (auto it = lambda->capture_begin(); it != lambda->capture_end();) {
            VarDecl *captured_var = it->getCapturedVar();
            cout << captured_var->getNameAsString() //TODO add f_
                 << "(" << captured_var->getNameAsString() << "_" << ")"
                 << ((++it != lambda->capture_end()) ? ", " : "");
        }
        cout << " {}" << endl;
        //Lambda class operator()
        FunctionDecl* lambdaFunction = lambda->getCallOperator();
        cout << indent << QualType::getAsString(lambdaFunction->getReturnType().getSplitDesugaredType())//XXX
             << " operator ()(";
        for (size_t i = 0; i != lambdaFunction->param_size();) {
            ParmVarDecl* parameter = lambdaFunction->getParamDecl(i);
            cout << parameter->getType().getAsString() << " "
                 << parameter->getQualifiedNameAsString()
                 << ((++i != lambdaFunction->param_size()) ? ", " : "");
        }
        cout << ") const ";
        cout << stringFromStmt(lambda->getBody(), f_context) << endl;
        //Lambda class operator() body
        //vector<string> body_lines = codeBlockToLines(stringFromStmt(lambda->getBody(), f_context));
        //for (string line : body_lines)
        //{
        //    cout << indent << line << endl;
        //}
        cout << "};" << endl;
        cout << "----------------" << endl;
        count++;
        return true;
    }

}