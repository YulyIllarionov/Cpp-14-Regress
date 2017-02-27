
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

    string GenericTypeGenerator::toString() {
        return std::string("type" + to_string(f_count));
    }

    string GenericTypeGenerator::generate() {
        f_count++;
        return toString();
    }

    //TODO generic
    //TODO incapture initialization
    bool LambdaFunctionReplacer::VisitLambdaExpr(LambdaExpr *lambda) {

        if (f_context->getSourceManager().isInSystemHeader(lambda->getLocStart()))
            return true;

        static int count = 0;
        //ast_graph ag(lambda, f_context);
        string dot_filename = "/home/yury/llvm-clang/test/dot/";
        //ag.to_dot_file(dot_filename + "lambda" + to_string(count) + ".dot");

        cout << "Lambda " << count << ": " << endl << "----------------" << endl;

        Indent indent;

        CXXRecordDecl *lambdaClass = lambda->getLambdaClass();
        DenseMap<const VarDecl *, FieldDecl *> clangCaptures;
        FieldDecl *thisField;
        lambdaClass->getCaptureFields(clangCaptures, thisField);
        //Lambda Class
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
        cout << indent << LambaClassNameGenerator::toString() << " (";
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
        FunctionDecl *lambdaFunction = lambda->getCallOperator();
        GenericTypeGenerator gtg;
        std::vector<string> typeNames;
        auto generics = lambda->getTemplateParameterList();
        if (generics) {
            cout << indent << "template <";
            for (size_t i = 0; i < generics->size();) {
                typeNames.push_back(gtg.generate());
                cout << "typename " << typeNames.back()
                     << ((++i != generics->size()) ? ", " : ">");
            }
            cout << endl;
        }
        cout << indent << QualType::getAsString(lambdaFunction->getReturnType().getSplitDesugaredType())//XXX
             << " operator() (";
        auto tn = typeNames.begin();
        for (size_t i = 0; i != lambdaFunction->param_size();) {
            ParmVarDecl *parameter = lambdaFunction->getParamDecl(i);
            cout << ((isa<TemplateTypeParmType>(*parameter->getType())) ? *tn++ :
                     parameter->getType().getAsString()) << " "
                 << parameter->getQualifiedNameAsString()
                 << ((++i != lambdaFunction->param_size()) ? ", " : "");
        }
        cout << ") const ";
        //Lambda class operator() body
        cout << toSting(lambda->getBody(), f_context) << endl;
        cout << "};" << endl;
        cout << "----------------" << endl;
        count++;
        return true;
    }

}
