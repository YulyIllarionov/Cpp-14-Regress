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
#include <set>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    string console_hline(char c) {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return string(w.ws_col, c);
    }

    string toString(SourceRange sr, const ASTContext &context, bool tokenEnd) {
        const SourceManager &sm = context.getSourceManager();
        const LangOptions &lo = context.getLangOpts();
        const char *b = sm.getCharacterData(sr.getBegin());
        SourceLocation end = sr.getEnd();
        if (tokenEnd)
            end = Lexer::getLocForEndOfToken(sr.getEnd(), 0, sm, lo);
        const char *e = sm.getCharacterData(end);
        return std::string(b, e);
    }

    bool RecursiveVariableReplacer::VisitDeclRefExpr(DeclRefExpr *dre) {
        if (dre->getDecl()->getNameAsString() == f_variable->getNameAsString()) {
            f_rewriter->ReplaceText(SourceRange(dre->getLocStart(),
                                                dre->getLocEnd()), f_generator->toString());
        }
        return true;
    }

    bool isCppFile(const Twine &path) {
        string filename(path.str());
        if ((filename.substr(filename.size() - 4, 4) == ".cpp"))
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

    bool isCppSourceFile(string filename) {
        if ((string(filename.end() - 4, filename.end()) == ".cpp") ||
            (string(filename.end() - 2, filename.end()) == ".c"))
            return true;
        return false;
    }

    SourceRange getParamRange(const FunctionDecl *func, const ASTContext &context) {
        SourceRange range;
        if (!func)
            return range;

        SourceLocation sl = func->getLocation();
        SourceLocation bodyBegin;
        const SourceManager &sm = context.getSourceManager();
        if (func->hasBody())
            bodyBegin = func->getBody()->getLocStart();
        else
            bodyBegin = func->getLocEnd();
        int l = sm.getCharacterData(bodyBegin) - sm.getCharacterData(sl);
        int i = 1;
        for (; i < l; i++) { //TODO fix getLocationAfterToken
            sl = Lexer::findLocationAfterToken(
                    sl, tok::TokenKind::l_paren,
                    context.getSourceManager(),
                    context.getLangOpts(), false);
            if (sl.isValid())
                break;
            else
                sl = func->getLocation().getLocWithOffset(i);
        }
        range.setBegin(sl.getLocWithOffset(-1));
        for (; i < l; i++) {
            sl = Lexer::findLocationAfterToken(
                    sl, tok::TokenKind::r_paren,
                    context.getSourceManager(),
                    context.getLangOpts(), false);
            if (sl.isValid())
                break;
            else
                sl = func->getLocation().getLocWithOffset(i);
        }
        range.setEnd(sl.getLocWithOffset(-1));
        return range;
    }

    vector<string> pathToVector(std::string path) {
        vector<string> pathVector;
        if (path.empty())
            return pathVector;
        if (path.front() == '/')
            path.erase(0, 1);
        if (path.back() == '/')
            path.pop_back();
        if (path.empty())
            return pathVector;
        string::size_type begin = 0;
        bool next = true;
        while (next) {
            string::size_type end = path.find('/', begin);
            if (end == string::npos) {
                next = false;
                end = path.size();
            }
            pathVector.push_back(path.substr(begin, end - begin));
            begin = ++end;
        }
        return pathVector;
    }

    std::string pathPopBack(std::string &path) {
        if (path.empty())
            return string();
        if (path.back() == '/')
            path.pop_back();
        string::size_type pos = path.find_last_of('/');
        if ((pos == string::npos) || (pos == (path.size() - 1)))
            return string();
        string file(path.substr(++pos));
        path.erase(pos);
        return file;
    }

    std::string removeExtension(const std::string &path) {
        return string(path.begin(), find(path.begin(), path.end(), '.'));
    }

    std::string asFolder(const std::string &path) {
        if (path.empty())
            return string();
        string folder(path);
        if (folder.back() != '/')
            folder += '/';
        return folder;
    }

    std::string NameGenerator::generate() {
        if (f_first)
            f_first = false;
        else
            f_count++;
        return toString();
    }

    void NameGenerator::reset() {
        f_count = 0;
        f_first = true;
    }

    SourceLocation getIncludeLocation(FileID fileID, SourceManager &sm, unsigned carriages) {
        set<unsigned> lines;
        if (fileID.isInvalid())
            return SourceLocation();
        for (auto it = sm.fileinfo_begin(); it != sm.fileinfo_end(); it++) {
            SourceLocation includeLoc = sm.getIncludeLoc(sm.translateFile(it->first));
            if (includeLoc.isValid() && sm.isInFileID(includeLoc, fileID)) {
                lines.insert(sm.getSpellingLineNumber(includeLoc));
            }
        }
        unsigned pos(0);
        if (!lines.empty()) {
            bool first = true;
            for (unsigned line :lines) {
                if (first)
                    first = false;
                else if ((line - pos) > carriages)
                    break;
                pos = line;
                //cout << "Include line:" << pos << endl;
            }
            //cout << console_hline('-') << endl;
        }
        cout << sm.getFileEntryForID(fileID)->getName() << endl;
        return sm.translateFileLineCol(sm.getFileEntryForID(fileID), ++pos, 1);
    }

    vector<string> getIncludes(FileID fileID, const ASTContext &context) {
        const SourceManager &sm = context.getSourceManager();
        const LangOptions &lo = context.getLangOpts();
        vector<string> includes;
        for (auto it = sm.fileinfo_begin(); it != sm.fileinfo_end(); it++) {
            SourceLocation includeBegin = sm.getIncludeLoc(sm.translateFile(it->first));
            if (includeBegin.isValid() && sm.isInFileID(includeBegin, fileID)) {
                SourceLocation includeEnd = Lexer::getLocForEndOfToken(includeBegin, 0, sm, lo);
                includeEnd = Lexer::getLocForEndOfToken(includeEnd, 0, sm, lo); //TODO fix
                string include = toString(SourceRange(includeBegin, includeEnd), context);
                if (find(includes.begin(), includes.end(), include) == includes.end())
                    includes.push_back(include);
            }
        }
        return includes;
    }

    SourceLocation findTokenLoc(SourceRange sr, const ASTContext &context,
                                tok::TokenKind kind, unsigned size) {
        if (sr.isInvalid())
            return SourceLocation();
        const SourceManager &sm = context.getSourceManager();
        const LangOptions &lo = context.getLangOpts();

        int l = sm.getCharacterData(sr.getEnd()) - sm.getCharacterData(sr.getBegin());
        SourceLocation sl;
        for (int i = 0; i < l; i++) { //TODO fix getLocationAfterToken
            sl = sr.getBegin().getLocWithOffset(i);
            sl = Lexer::findLocationAfterToken(sl, kind, sm, lo, false);
            if (sl.isValid())
                break;
        }
        return sl.getLocWithOffset(-size);
    }
}
