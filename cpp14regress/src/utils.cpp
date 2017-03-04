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
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"

#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool RecursiveVariableReplacer::VisitDeclRefExpr(DeclRefExpr *dre) {
        if (dre->getDecl()->getNameAsString() == f_variable->getNameAsString()) {
            f_rewriter->ReplaceText(SourceRange(dre->getLocStart(),
                                                dre->getLocEnd()), f_generator->toString());
        }
        return true;
    }

    Indent &Indent::operator++() {
        f_level++;
        return *this;
    }

    Indent Indent::operator++(int) {
        Indent tmp(*this);
        ++(*this);
        return tmp;
    }

    Indent &Indent::operator--() {
        if (f_level)
            f_level--;
        return *this;
    }

    Indent Indent::operator--(int) {
        Indent tmp(*this);
        --(*this);
        return tmp;
    }

    ostream &operator<<(ostream &stream, const Indent &indent) {
        for (unsigned int i = 0; i < indent.f_size * indent.f_level; i++)
            stream << ' ';
        return stream;
    }

    vector<string> filesInFolder(string folder) {
        vector<string> filenames;
        error_code ec;
        for (sys::fs::recursive_directory_iterator i(Twine(folder), ec), e;
             i != e; i.increment(ec)) {
            if (!sys::fs::is_directory(i->path()) &&
                sys::fs::can_write(i->path()) &&
                isCppFile(i->path()))
                filenames.push_back(i->path());
        }
        return filenames;
    }

    bool isCppFile(string filename) {
        if ((string(filename.end() - 4, filename.end()) == ".cpp") ||
            (string(filename.end() - 4, filename.end()) == ".hpp") ||
            (string(filename.end() - 2, filename.end()) == ".c") ||
            (string(filename.end() - 2, filename.end()) == ".h"))
            return true;
        return false;
    }
}
