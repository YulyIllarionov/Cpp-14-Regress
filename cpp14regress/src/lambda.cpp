
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

#include "lambda.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

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

    string GenericTypeGenerator::toString() {
        return std::string("type" + to_string(f_count));
    }

    string GenericTypeGenerator::generate() {
        f_count++;
        return toString();
    }

    LambdaFunctionReplacer::LambdaFunctionReplacer(ASTContext *context, cpp14features_stat *stat)
            : f_context(context), f_stat(stat) {
        f_rewriter = new Rewriter(context->getSourceManager(),
                                  context->getLangOpts());
    }



    //TODO generic
    //TODO incapture initialization
    bool LambdaFunctionReplacer::VisitLambdaExpr(LambdaExpr *lambda) {
        if (!inProcessedFile(lambda, f_context))
            return true;

        cout << f_context->getSourceManager().getFileLoc(lambda->getLocStart()).
                printToString(f_context->getSourceManager()) << endl;

        stringstream header;
        header << "//Lambda at:"
               << lambda->getLocStart().printToString(f_context->getSourceManager())<<  endl;

        Indent indent;

        CXXRecordDecl *lambdaClass = lambda->getLambdaClass();
        DenseMap<const VarDecl *, FieldDecl *> clangCaptures;
        FieldDecl *thisField;
        lambdaClass->getCaptureFields(clangCaptures, thisField);
        //Lambda Class
        header << "class " << LambaClassNameGenerator::generate() << " {" << endl;
        if (lambda->capture_begin() != lambda->capture_end())
            header << "private:" << endl;
        //Lambda class fields
        ++indent;
        for (auto it = lambda->captures().begin(); it != lambda->captures().end(); it++) {
            VarDecl *captured_var = it->getCapturedVar();
            header << indent << ((lambda->isMutable()) ? "mutable " : "")
                 << clangCaptures[captured_var]->getType().getAsString() << " "
                 << captured_var->getNameAsString() << ";" << endl; //TODO add f_
        }
        header << "public:" << endl;
        //Lambda class constructor
        header << indent << LambaClassNameGenerator::toString() << " (";
        for (auto it = lambda->capture_begin(); it != lambda->capture_end();) {
            VarDecl *captured_var = it->getCapturedVar();
            header << clangCaptures[captured_var]->getType().getAsString() << " "
                 << captured_var->getNameAsString() << "_"
                 << ((++it != lambda->capture_end()) ? ", " : ") : ");
        }
        //Lambda class constructor body
        for (auto it = lambda->capture_begin(); it != lambda->capture_end();) {
            VarDecl *captured_var = it->getCapturedVar();
            header << captured_var->getNameAsString() //TODO add f_
                 << "(" << captured_var->getNameAsString() << "_" << ")"
                 << ((++it != lambda->capture_end()) ? ", " : "");
        }
        header << " {}" << endl;
        //Lambda class operator()1111
        FunctionDecl *lambdaFunction = lambda->getCallOperator();
        GenericTypeGenerator gtg;
        std::vector<string> typeNames;
        auto generics = lambda->getTemplateParameterList();
        if (generics) {
            header << indent << "template <";
            for (size_t i = 0; i < generics->size();) {
                typeNames.push_back(gtg.generate());
                header << "typename " << typeNames.back()
                     << ((++i != generics->size()) ? ", " : ">");
            }
            header << endl;
        }
        header << indent << QualType::getAsString(lambdaFunction->getReturnType().getSplitDesugaredType())//XXX
             << " operator() (";
        auto tn = typeNames.begin();
        for (size_t i = 0; i != lambdaFunction->param_size();) {
            ParmVarDecl *parameter = lambdaFunction->getParamDecl(i);
            header << ((isa<TemplateTypeParmType>(*parameter->getType())) ? *tn++ :
                     parameter->getType().getAsString()) << " "
                 << parameter->getQualifiedNameAsString()
                 << ((++i != lambdaFunction->param_size()) ? ", " : "");
        }
        header << ") const ";
        //Lambda class operator() body
        header << toSting(lambda->getBody(), f_context) << endl;
        header << "};" << endl;
        header << endl;
        //header.flush();
        ofstream header_file;
        static bool first = true;
        if (first) {
            header_file.open(f_header_path);
            first = false;
        }
        else
            header_file.open(f_header_path, fstream::app);
        if (!header_file.is_open()) {
            cerr << "Can not open " << f_header_path << " file" << endl;
            return false;
        }
        header_file << header.str();
        header_file.close();
        return true;
    }

}
