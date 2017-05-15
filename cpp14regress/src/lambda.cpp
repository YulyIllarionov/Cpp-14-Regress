
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

    //TODO check Types
    //TODO capture this
    bool LambdaReplacer::VisitLambdaExpr(LambdaExpr *lambda) {
        if (fromSystemFile(lambda, astContext()))
            return true;

        f_header << "//Lambda at:"
                 << lambda->getLocStart().printToString(astContext().getSourceManager()) << endl;

        CXXRecordDecl *lambdaClass = lambda->getLambdaClass();
        DenseMap<const VarDecl *, FieldDecl *> clangCaptures;
        FieldDecl *thisField;
        lambdaClass->getCaptureFields(clangCaptures, thisField);

        //Lambda Class
        f_header << "class " << f_lcng.generate() << " {" << endl;
        if (lambda->capture_begin() != lambda->capture_end())
            f_header << "private:" << endl;

        //Lambda class fields
        for (auto it = lambda->captures().begin(); it != lambda->captures().end(); it++) {
            VarDecl *captured_var = it->getCapturedVar();
            f_header << ((lambda->isMutable()) ? "mutable " : "")
                     << clangCaptures[captured_var]->getType().getAsString() << " "
                     << VariableToField(captured_var->getNameAsString()) << ";" << endl;
        }

        f_header << "public:" << endl;
        //Lambda class constructor
        f_header << f_lcng.toString() << " (";
        for (auto it = lambda->capture_begin(); it != lambda->capture_end();) {
            VarDecl *captured_var = it->getCapturedVar();
            f_header << clangCaptures[captured_var]->getType().getAsString() << " "
                     << captured_var->getNameAsString()
                     << ((++it != lambda->capture_end()) ? ", " : "");
        }
        f_header << ") ";
        if (lambda->capture_begin() != lambda->capture_end())
            f_header << ": ";
        //Lambda class constructor body
        for (auto it = lambda->capture_begin(); it != lambda->capture_end();) {
            VarDecl *captured_var = it->getCapturedVar();
            f_header << VariableToField(captured_var->getNameAsString())
                     << "(" << captured_var->getNameAsString() << ")"
                     << ((++it != lambda->capture_end()) ? ", " : "");
        }
        f_header << " {}" << endl;

        //Lambda class operator()
        FunctionDecl *lambdaFunction = lambda->getCallOperator();
        GenericTypeGenerator gtg;
        std::vector<string> typeNames;
        auto generics = lambda->getTemplateParameterList();
        if (generics) {
            f_header << "template <";
            for (size_t i = 0; i < generics->size();) {
                typeNames.push_back(gtg.generate());
                f_header << "typename " << typeNames.back()
                         << ((++i != generics->size()) ? ", " : ">");
            }
            f_header << endl;
        }
        f_header << lambdaFunction->getReturnType().getAsString(PrintingPolicy(langOptions()))
                 << " operator() (";
        auto tn = typeNames.begin();
        for (size_t i = 0; i != lambdaFunction->param_size();) {
            ParmVarDecl *parameter = lambdaFunction->getParamDecl(i);
            f_header << ((isa<TemplateTypeParmType>(*parameter->getType())) ? *tn++ :
                         parameter->getType().getAsString()) << " "
                     << parameter->getQualifiedNameAsString()
                     << ((++i != lambdaFunction->param_size()) ? ", " : "");
        }
        f_header << ") const ";
        //Lambda class operator() body
        f_header << toString(lambda->getBody(), astContext()) << endl;
        f_header << "};" << endl;

        //Lambda class call
        string call(f_lcng.toString() + "(");
        for (auto it = lambda->capture_init_begin(); it != lambda->capture_init_end();) {
            auto capture = *it;
            if (!capture) {
                //TODO
                return false;
            }
            call += toString(capture, astContext());
            call += ((++it != lambda->capture_init_end()) ? ", " : "");
        }
        call += ")";
        rewriter()->ReplaceText(lambda->getSourceRange(), call);

        return true;
    }

    //TODO add include to all files;
    void LambdaReplacer::endSourceFileAction() {
        FileID fileID = sourceManager().getMainFileID();
        SourceLocation includeLoc = getIncludeLocation(fileID, sourceManager());
        if (includeLoc.isInvalid()) {
            //TODO
            return;
        }
        string include("#include \"");
        include += f_lhng.generate();
        include += "\"\n";
        rewriter()->InsertText(includeLoc, include);

        string folder = asFolder(sourceManager().getFileEntryForID(fileID)->getDir()->getName());
        std::error_code ec;
        ofstream header(folder + f_lhng.toString());
        if (!header.is_open()) {
            cerr << "Can not create header \"" << folder + f_lhng.generate()
                 << "\" for lambda" << endl;
            return;
        }
        header << "#ifndef " << f_lhgg.generate() << endl
               << "#define " << f_lhgg.toString() << endl << endl;
        //vector<string> includes = getIncludes(sourceManager().getMainFileID(), astContext());
        //for (string include : includes)
        //    header << "#include " << include << endl;
        //header << endl;
        header << f_header.rdbuf() << endl
               << "#endif" << endl;
        header.close();
    }


    /*LambdaFunctionReplacer::LambdaFunctionReplacer(ASTContext *context, cpp14features_stat *stat,
                                                   DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(),
                                  context->getLangOpts());
        SourceManager &sm = f_context->getSourceManager();
        cout << console_hline('+') << endl;
        for (auto it = sm.fileinfo_begin(); it != sm.fileinfo_end(); it++) {
            cout << it->first->getName() << endl;
        }
        cout << console_hline('+') << endl;
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
               << lambda->getLocStart().printToString(f_context->getSourceManager()) << endl;

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
        header << indent << QualType::getAsString(
                lambdaFunction->getReturnType().getSplitDesugaredType())//XXX
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
        header << toString(lambda->getBody(), f_context) << endl;
        header << "};" << endl;
        header.flush();
        ofstream header_file;
        static bool first = true;
        if (first) {
            header_file.open(f_header_path);
            first = false;
        } else
            header_file.open(f_header_path, fstream::app);
        if (!header_file.is_open()) {
            cerr << "Can not open " << f_header_path << " file" << endl;
            return false;
        }
        header_file << header.str() << endl;
        header_file.close();

        //cout << console_hline('-') << endl << header.str() << console_hline('.') << endl;
//
        //for (auto it = lambda->capture_init_begin(); it != lambda->capture_init_end(); it++) {
        //    cout << toString(*it, f_context) << endl;
        //}
//
        //cout << console_hline('.') << endl;
//
        //for (auto it = lambda->capture_begin(); it != lambda->capture_end(); it++) {
        //    cout << it->getCapturedVar()->getNameAsString() << " -- "
        //         << lambda->isInitCapture(it) << endl;
        //}
        //cout << console_hline('-') << endl;
        //header_file.close();
        return true;
    }*/


}
