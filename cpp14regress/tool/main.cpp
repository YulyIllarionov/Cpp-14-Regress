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
#include "clang/Tooling/JSONCompilationDatabase.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdarg>
#include <sys/ioctl.h>
#include <unistd.h>

#include "base_types.h"
#include "cpp14_scanner.h"
#include "default.h"
#include "auto.h"
#include "decltype.h"
#include "lambda.h"
#include "constuctor_delegation.h"

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace cpp14regress;

std::string console_hline() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return string(w.ws_col, '*');
}

static cl::OptionCategory MyToolCategory("");

//TODO спросить про отображние стека вызовов

int main(int argc, const char **argv) {

    if (argc != 2) {
        cerr << "error: wrong argument number" << endl;
        return 1;
    }

    Twine srcDir(argv[1]);
    if (!sys::fs::is_directory(argv[1])) {
        cerr << "error: second argument is not a folder" << endl;
        return 2;
    }

    cpp14features_stat stat;
    Cpp14RegressFrontendActionFactory<Cpp14scanner> factory(&stat);
    int result;

    string em;
    unique_ptr<CompilationDatabase> cb =
            CompilationDatabase::loadFromDirectory(srcDir.str(), em);
    if (!cb) {
        std::error_code ec;
        vector<string> argv_tmp{argv[0]};
        for (sys::fs::recursive_directory_iterator i(srcDir, ec), e; i != e; i.increment(ec)) {
            if (isCppSourceFile(i->path()))
                argv_tmp.push_back(i->path());
        }
        cout << "Running tool from source files" << endl;
        cout << console_hline() << endl;
        for (auto it = argv_tmp.begin() + 1; it != argv_tmp.end(); it++)
            cout << *it << endl;

        argv_tmp.push_back("--");
        argv_tmp.push_back("-std=c++14");
        int argc_mod = argv_tmp.size();
        char **argv_mod = new char *[argc_mod];
        for (int i = 0; i < argc_mod; i++) {
            argv_mod[i] = new char[argv_tmp[i].size() + 1];
            std::strcpy(argv_mod[i], argv_tmp[i].c_str());
        }
        CommonOptionsParser op(argc_mod, const_cast<const char **>(argv_mod), MyToolCategory);
        ClangTool Tool(op.getCompilations(), op.getSourcePathList());
        cout << console_hline() << endl;
        cout << "Press enter to continue";
        getchar();
        result = Tool.run(&factory);
    } else {
        cout << "Running tool from compilation database" << endl;
        cout << console_hline() << endl;
        for (auto file: cb->getAllFiles())
            cout << file << endl;
        ClangTool Tool(*cb, cb->getAllFiles());
        cout << console_hline() << endl;
        cout << "Press enter to continue" << endl;
        getchar();
        result = Tool.run(&factory);
    }

    cout << console_hline() << endl;
    for (int i = (int) cpp14features::begin; i < (int) cpp14features::end; i++)
        if (stat.size((cpp14features) i) != 0)
            cout << stat.toString((cpp14features) i) << " -- "
                 << stat.size((cpp14features) i) << endl;
    cout << console_hline() << endl;

    return result;
}
