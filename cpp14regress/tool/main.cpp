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

#include "default.h"
#include "delete.h"
#include "auto.h"
#include "decltype.h"
//#include "lambda.h"
#include "constuctor_delegation.h"
#include "raw_string.h"
#include "implict_sizeof.h"
#include "digit_separators.h"
#include "alias_type.h"
#include "uniform_initialization.h"
#include "explicit_conversion.h"
#include "in_class_init.h"
#include "range_based_for.h"
//#include "strongly_typed_enum.h"
#include "user_literals.h"
#include "final.h"
#include "override.h"
#include"binary_literals.h"
#include "list_init.h"
#include "static_assert.h"
#include "trailing_return.h"
#include "func_templ_default_args.h"
#include "nullptr.h"
#include "alignof.h"
#include "alignas.h"
#include "unicode_literals.h"
#include "inline_namespace.h"
#include "unrestricted_unions.h"
#include "variable_template.h"

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

    cout << "Running tool on: " << sourceFile << endl;

    const char *argv_mod[] = {argv[0], sourceFile.data(), "--", "-std=c++14"};
    int argc_mod = 4;
    CommonOptionsParser op(argc_mod, argv_mod, MyToolCategory);
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    int result = Tool.run(
            newFrontendActionFactory<FeatureReplacerFrontendAction<VariableTemplateSearcher>>().get());

    return result;
}
