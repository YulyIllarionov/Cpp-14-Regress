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
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdarg>

#include "base_types.h"

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace cpp14regress;

static cl::OptionCategory MyToolCategory("");

int main(int argc, const char **argv) {

    vector<string> filenames = filesInFolder(
            "/home/yury/llvm-clang/llvm/tools/clang/tools/extra/cpp14regress_ide/cpp14regress/");

    for (string file : filenames)
        cout << file << endl;

    vector<string> argv_tmp(argv, argv + argc);
    cout << endl;
    for (int i = 1; argv_tmp[i] != "--"; i++)
    {
        argv_tmp[i].insert(argv_tmp[i].find_last_of('.'), "_regressed");
        cout << "file №" << i << ": " << argv_tmp[i] << endl;
        ifstream  src(argv[i], std::ios::binary);
        if (src.is_open()) {
            ofstream dst(argv_tmp[i], std::ios::binary);
            dst << src.rdbuf();
            src.close();
            dst.close();
        }
    }
    cout << endl;
    char** argv_mod = new char*[argc];
    for(int i = 0; i < argc; i++)
    {
        argv_mod[i] = new char[argv_tmp[i].size() + 1];
        std::strcpy(argv_mod[i], argv_tmp[i].c_str());
    }
    CommonOptionsParser op(argc, const_cast<const char**>(argv_mod), MyToolCategory);
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    //int result = Tool.run((newFrontendActionFactory<RangeBasedForFrontendAction>()).get());

    //std::error_code EC;
    //raw_fd_ostream *cured = new raw_fd_ostream(curedFilename, EC, sys::fs::F_Text);
    //cured->close();

    //return result;
    return 0;
}
