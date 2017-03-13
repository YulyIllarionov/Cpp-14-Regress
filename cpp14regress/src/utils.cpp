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

#include "utils.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;

    bool RecursiveVariableReplacer::VisitDeclRefExpr(DeclRefExpr *dre) {
        if (dre->getDecl()->getNameAsString() == f_variable->getNameAsString()) {
            f_rewriter->ReplaceText(SourceRange(dre->getLocStart(),
                                                dre->getLocEnd()), f_generator->toString());
        }
        return true;
    }

    bool isCppFile(const Twine &path) {
        string filename(path.str());
        if ((filename.substr(filename.size() - 4, 4) == ".cpp")
            /*|| (filename.substr(filename.size() - 2, 2) == ".h")*/)
            return true;
        return false;
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
}