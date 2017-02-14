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

    std::string stringFromStmt(clang::Stmt *stmt, clang::ASTContext *context) {
        const SourceManager &sm = context->getSourceManager();
        const LangOptions &lo = context->getLangOpts();
        SourceLocation b(stmt->getLocStart()), _e(stmt->getLocEnd());
        SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, lo));
        return string(sm.getCharacterData(b),
                      sm.getCharacterData(e) - sm.getCharacterData(b));
    }

    std::string stringFromDecl(clang::Decl *decl, clang::ASTContext *context) {
        const SourceManager &sm = context->getSourceManager();
        const LangOptions &lo = context->getLangOpts();
        SourceLocation b(decl->getLocStart()), _e(decl->getLocEnd());
        SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, lo));
        return string(sm.getCharacterData(b),
                      sm.getCharacterData(e) - sm.getCharacterData(b));
    }

    Indent& Indent::operator++() {
        f_level++;
        return *this;
    }

    Indent Indent::operator++(int)
    {
        Indent tmp(*this);
        ++(*this);
        return tmp;
    }

    Indent& Indent::operator--() {
        if(f_level)
            f_level--;
        return *this;
    }

    Indent Indent::operator--(int)
    {
        Indent tmp(*this);
        --(*this);
        return tmp;
    }

    ostream& operator<<(ostream& stream, const Indent& indent) {
        for (unsigned int i = 0; i < indent.f_size * indent.f_level; i++)
            stream << ' ';
        return stream;
    }

    vector<string> codeBlockToLines(const string &block) //XXX
    {
        vector<string> lines;
        if(!block.empty()) {
            Indent indent;
            size_t b = block.find_first_not_of('\n') , e = b;
            while ((e = block.find('\n', ++e)) != string::npos) {
                lines.push_back(block.substr(b , e - b - 1));
                b = e;
                string &tmp_line = lines.back();
                tmp_line.erase(0, tmp_line.find_first_not_of('\t'));
                tmp_line.erase(0, tmp_line.find_first_not_of(' '));
                tmp_line.erase(tmp_line.find_last_not_of(' ') + 1);
                tmp_line.erase(tmp_line.find_last_not_of('\t') + 1);

                //if (tmp_line.empty())
                //    cout << endl;
                //else {
                //    size_t brace;
                //    if ((brace = tmp_line.find('{')) != string::npos) {
                //        if (brace == 0)
                //            tmp_line = --indent + tmp_line;
                //        else
                //            tmp_line = indent-- + tmp_line;
                //    }
                //    if ((brace = tmp_line.find('}')) != string::npos) {
                //        if (brace == 0)
                //            tmp_line = --indent + tmp_line;
                //        else
                //            tmp_line = indent-- + tmp_line;
                //    }
                //
                //    } else {
                //        cout << indent << line << endl;
                //    }
                //}
            }
        }
        return lines;
    }
}