
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
#include "llvm/ADT/DenseMap.h"

#include "raw_string.h"
#include "utils.h"

#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    RawStringReplacer::RawStringReplacer(ASTContext *context, cpp14features_stat *stat, DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void RawStringReplacer::EndFileAction() {
        for (auto i = f_rewriter->buffer_begin(), e = f_rewriter->buffer_end(); i != e; ++i) {
            const FileEntry *entry = f_context->getSourceManager().getFileEntryForID(i->first);
            string file = f_dg->getFile(entry->getName());
            std::error_code ec;
            sys::fs::remove(Twine(file));
            raw_fd_ostream rfo(StringRef(file), ec, sys::fs::OpenFlags::F_Excl | sys::fs::OpenFlags::F_RW);
            //cout << "Trying to write " << entry->getName() << " to " << file << " with " << ec.message() << endl;
            i->second.write(rfo);
        }
    }

    bool RawStringReplacer::VisitStringLiteral(clang::StringLiteral *literal) {
        if (!inProcessedFile(literal, f_context))
            return true;

        string sl = toSting(literal, f_context);
        string::size_type pos = sl.find('\"');
        if ((pos != 0) && (pos != string::npos)) {
            if (sl[--pos] == 'R') {
                string raw = string(literal->getBytes());
                for (size_t i = 0; i < raw.size(); i++) {
                    string s;
                    if (escapeCharToString(raw[i], s)) {
                        raw.replace(i, 1, s);
                        //cout << "Ecape at " << i << " is " << s << endl;
                        i++;
                    }
                }
                cout << raw << endl;
            }
        }
        return true;
    }

    bool RawStringReplacer::escapeCharToString(char c, string& s) {
        switch (c) {
            case '\'' : { s = "\\\'"; break; }
            case '\"' : { s = "\\\""; break; }
            case '\?' : { s = "\\?"; break; }
            case '\\' : { s = "\\\\"; break; }
            case '\a' : { s = "\\a"; break; }
            case '\b' : { s = "\\b"; break; }
            case '\f' : { s = "\\f"; break; }
            case '\n' : { s = "\\n"; break; }
            case '\r' : { s = "\\r"; break; }
            case '\t' : { s = "\\t"; break; }
            case '\v' : { s = "\\v"; break; }
            default : return false;
        }
        return true;
    }
}
