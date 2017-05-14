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
#include <clang/Sema/Ownership.h>

#include "base_types.h"
#include "utils.h"
#include "cpp14_scanner.h"
#include "default.h"
#include "auto.h"
#include "decltype.h"
#include "lambda.h"
#include "constuctor_delegation.h"
#include "raw_string.h"
#include "implict_sizeof.h"
#include "digit_separators.h"
#include "alias_type.h"
#include "uniform_initialization.h"
#include "explicit_conversion.h"
#include "in_class_init.h"
#include "range_based_for.h"
#include "strongly_typed_enum.h"
#include "user_literals.h"
#include "final.h"
#include "file_preparator.h"

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace cpp14regress;

static cl::OptionCategory MyToolCategory("");

//typedef FilesPreparator ToolType;

int main(int argc, const char **argv) {

    if (argc != 3) {
        cerr << "error: wrong argument number, need 2" << endl;
        return 1;
    }

    Twine dstDir(argv[2]);
    if (!sys::fs::is_directory(argv[2])) {
        cerr << "error: second argument \"" << argv[2] << "\" is not a folder" << endl;
        return 2;
    }

    string em;
    unique_ptr<CompilationDatabase> cb;
    Twine srcPath(argv[1]);
    string srcDir = srcPath.str();
    if (!sys::fs::is_directory(srcPath.str()))
        pathPopBack(srcDir);

    cb = CompilationDatabase::loadFromDirectory(srcDir, em);

    vector<string> sourceFiles;
    FilesPreparatorFactory preparatorFactory(dstDir.str(), &sourceFiles);

    if (!cb) {
        std::error_code ec;
        vector<string> argv_tmp{argv[0]};
        if (sys::fs::is_directory(srcPath.str())) {
            for (sys::fs::recursive_directory_iterator i(srcPath, ec), e; i != e; i.increment(ec)) {
                if (isCppSourceFile(i->path()))
                    argv_tmp.push_back(i->path());
            }
        } else {
            if (isCppSourceFile(srcPath.str()))
                argv_tmp.push_back(srcPath.str());
        }
        cout << "Running tool from source files" << endl;
        //cout << console_hline() << endl;
        //for (auto it = argv_tmp.begin() + 1; it != argv_tmp.end(); it++)
        //    cout << *it << endl;
        argv_tmp.push_back("--");
        argv_tmp.push_back("-std=c++14");
        int argc_mod = argv_tmp.size();
        char **argv_mod = new char *[argc_mod];
        for (int i = 0; i < argc_mod; i++) {
            argv_mod[i] = new char[argv_tmp[i].size() + 1];
            std::strcpy(argv_mod[i], argv_tmp[i].c_str());
        }
        CommonOptionsParser op(argc_mod, const_cast<const char **>(argv_mod), MyToolCategory);
        cb.reset(&op.getCompilations());
        ClangTool Tool(*cb, op.getSourcePathList());
        //cout << console_hline() << endl;
        cout << "Press enter to continue";
        getchar();
        Tool.run(&preparatorFactory); //TODO check result
    } else {
        cout << "Running tool from compilation database" << endl;
        //cout << console_hline() << endl;
        //for (auto file: cb->getAllFiles())
        //    cout << file << endl;
        ClangTool Tool(*cb, cb->getAllFiles());
        //cout << console_hline() << endl;
        cout << "Press enter to continue" << endl;
        getchar();
        Tool.run(&preparatorFactory); //TODO check result
    }

    //ClangTool Tool(*cb, sourceFiles);
    //Tool.run(newFrontendActionFactory<FeatureReplacerFrontendAction<LambdaReplacer>>().get());

    //cout << console_hline() << endl;
    //for (int i = (int) cpp14features::begin; i < (int) cpp14features::end; i++)
    //    if (stat.size((cpp14features) i) != 0)
    //        cout << stat.toString((cpp14features) i) << " -- "
    //             << stat.size((cpp14features) i) << endl;
    //cout << console_hline() << endl;

    return 0;
}
