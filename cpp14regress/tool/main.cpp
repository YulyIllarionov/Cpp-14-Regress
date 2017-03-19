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
#include "cpp14_scanner.h"

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace cpp14regress;

static cl::OptionCategory MyToolCategory("");

// слияние AST -- ASTMergeAction
// как работать с хедерами?
// handleBeginSource и BeginSourceFileAction

//json


int main(int argc, const char **argv) {

    if (argc != 2) {
        cerr << "error: wrong arguments count, need 2." << endl;
        return 1;
    }

    Twine dir = argv[1];

    if (!sys::fs::is_directory(dir)) {
        cerr << "error: second argument is not a folder" << endl;
        return 2;
    }

    vector<string> argv_tmp{argv[0]};
    std::error_code ec;
    for (sys::fs::recursive_directory_iterator i(dir, ec), e; i != e; i.increment(ec)) {
        if (isCppFile(i->path()))
            argv_tmp.push_back(i->path());
    }
    argv_tmp.push_back("--");
    argv_tmp.push_back("-std=c++14");
    //for (int i = 1; argv_tmp[i] != "--"; i++) {
    //    argv_tmp[i].insert(argv_tmp[i].find_last_of('.'), "_regressed");
    //    cout << "file №" << i << ": " << argv_tmp[i] << endl;
    //    ifstream src(argv[i], std::ios::binary);
    //    if (src.is_open()) {
    //        ofstream dst(argv_tmp[i], std::ios::binary);
    //        dst << src.rdbuf();
    //        src.close();
    //        dst.close();
    //    }
    //}

    int argc_mod = argv_tmp.size();
    char **argv_mod = new char *[argc_mod];
    for (int i = 0; i < argc_mod; i++) {
        argv_mod[i] = new char[argv_tmp[i].size() + 1];
        std::strcpy(argv_mod[i], argv_tmp[i].c_str());
    }

    for (int i = 0 ; i < argc_mod; i++)
        cout << argv_mod[i] << endl;

    CommonOptionsParser op(argc_mod, const_cast<const char **>(argv_mod), MyToolCategory);
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    cpp14features_stat stat;

    Cpp14RegressFrontendActionFactory<Cpp14scanner> factory(&stat);

    int result = Tool.run(&factory);

    cout << "---------------" << endl;
    for (int i = (int) cpp14features::begin; i < (int) cpp14features::end; i++)
        if (stat.size((cpp14features) i) != 0)
            cout << stat.toString((cpp14features) i) << " -- "
                 << stat.size((cpp14features) i) << endl;
    cout << "---------------" << endl;

    ////std::error_code EC;
    ////raw_fd_ostream *cured = new raw_fd_ostream(curedFilename, EC, sys::fs::F_Text);
    ////cured->close();

    return result;
}
