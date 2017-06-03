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

#include "cpp14feature.h"
#include <string>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    namespace replacement {
        string info(features::type f, result r) {
            return string(seed + " " + features::toString(f) + " " + resultStrings[(int) r]);
        }

        string begin(features::type f, result r) { return info(f, r) + " begin"; }

        string end(features::type f, result r) { return info(f, r) + " end"; }
    }

    string Comment::line(const string &text) {
        string commented(text);
        commented.insert(0, "//");
        for (string::size_type i = 0; i < commented.size(); i++) {
            if (commented[i] == '\n') {
                commented.insert(++i, "//");
                i++;
            }
        }
        return commented;
    }

    string Comment::block(const string &text) {
        return string("/*" + text + "*/");
    }

}