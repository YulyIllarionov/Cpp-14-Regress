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

static cl::OptionCategory MyToolCategory("");

//TODO спросить про отображние стека вызовов

int main(int argc, const char **argv) {

    if (argc < 3) {
        cerr << "error: too few arguments" << endl;
        return 1;
    }

    std::error_code ec;

    string src_dir = argv[1];
    if (src_dir.back() != '/')
        src_dir += '/';
    Twine srcDir(src_dir);
    if (!sys::fs::is_directory(srcDir)) {
        cerr << "error: second argument is not a folder" << endl;
        return 2;
    }

    string dst_dir = argv[2];
    if (dst_dir.back() != '/')
        dst_dir += '/';
    Twine dstDir(dst_dir);
    if (sys::fs::exists(dstDir)) {
        if (!sys::fs::is_directory(dstDir)) {
            cerr << "error: third argument is not a folder" << endl;
            return 3;
        } else {
            for (sys::fs::directory_iterator i(dstDir, ec), e; i != e; i.increment(ec)) { //TODO recursive
                sys::fs::remove(i->path());
            }
        }
    } else {
        sys::fs::create_directory(dstDir);
    }

    size_t name_start = srcDir.str().size();
    for (sys::fs::recursive_directory_iterator i(srcDir, ec), e; i != e; i.increment(ec)) {
        if (isCppFile(i->path())) {
            //cout << dstDir.concat(i->path().substr(name_start)).str() << endl;
            sys::fs::copy_file(Twine(i->path()), dstDir.concat(i->path().substr(name_start)));
        }
    }
    return 0;

    vector<string> argv_tmp{argv[0]};
    for (sys::fs::recursive_directory_iterator i(dstDir, ec), e; i != e; i.increment(ec)) {
        if (isCppSourceFile(i->path()))
            argv_tmp.push_back(i->path());
    }

    //for (auto it = ++argv_tmp.begin(); it != argv_tmp.end(); it++)
    //    cout << "file " << *it << endl;


    argv_tmp.push_back("--");
    //argv_tmp.push_back("-p");
    //argv_tmp.push_back("/home/yury/llvm-clang/test/build");
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

    if (argc > 3) {
        //cout << "Compilation Database:" << argv[3] << endl;
        argv_tmp.insert(argv_tmp.begin() + 1, string(argv[3]));
        argv_tmp.insert(argv_tmp.begin() + 1, "-p");
    }

    for (string sp : argv_tmp)
        cout << sp << endl;

    int argc_mod = argv_tmp.size();
    char **argv_mod = new char *[argc_mod];
    for (int i = 0; i < argc_mod; i++) {
        argv_mod[i] = new char[argv_tmp[i].size() + 1];
        std::strcpy(argv_mod[i], argv_tmp[i].c_str());
    }

    CommonOptionsParser op(argc_mod, const_cast<const char **>(argv_mod), MyToolCategory);

    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    cpp14features_stat stat;

    Cpp14RegressFrontendActionFactory<ConstructorDelegationReplacer> factory(&stat);

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

    //int main(int argc, char **argv) {
    //  ClangTool Tool(argc, argv);
    //  MatchFinder finder;
    //  finder.AddMatcher(Id("id", record(hasName("::a_namespace::AClass"))),
    //                    new HandleMatch);
    //  return Tool.Run(newFrontendActionFactory(&finder));
    //}

    return result;
}
