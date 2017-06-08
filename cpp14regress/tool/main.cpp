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
#include "tool.h"

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;
using namespace cpp14regress;

static cl::OptionCategory MyToolCategory("");

int main(int argc, const char **argv) {

    if ((argc < 2) || (argc > 3)) {
        cerr << "error: wrong argument number" << endl;
        return 1;
    }

    string sourceFile(argv[1]);
    if (!sys::fs::is_regular_file(sourceFile)) {
        cerr << "error: first argument is not a file" << endl;
        return 2;
    }

    if (argc == 3) {
        string sourceDir(argv[2]);
        if (argc == 3) {
            if (!sys::fs::is_directory(sourceDir)) {
                cerr << "error: second argument is not a folder" << endl;
                return 3;
            }
            string sourceFileOrig = sourceFile;
            cout << sourceFileOrig << endl;
            sourceFile = asFolder(sourceDir) + pathPopBack(sourceFile);
            cout << sourceFile << endl;
            error_code ec = sys::fs::copy_file(sourceFileOrig, sourceFile);
            if (bool(ec)) {
                cerr << "error: " << ec.message() << endl;
                return 4;
            }
        }
    }

    cout << "Running tool on: " << sourceFile << endl
         << console_hline() << endl;
    const char *argv_mod[] = {argv[0], sourceFile.data(), "--", "-std=c++14"};
    int argc_mod = 4;
    CommonOptionsParser op(argc_mod, argv_mod, MyToolCategory);
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    for (unsigned i = 0; i < features::size(); i++) {
        features::type f = features::type(i);
        if (features::isSupported(f)) {
            cout << features::toString(f) << endl
                 << console_hline('-') << endl;
            auto faf = new Cpp14RegressFrontendActionFactory(f);
            Tool.run(faf);
            delete faf;
        }
    }
    return 0;
}
