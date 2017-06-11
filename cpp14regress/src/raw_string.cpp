
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

    bool RawStringReplacer::VisitStringLiteral(clang::StringLiteral *literal) {
        if (!fromUserFile(literal, f_sourceManager))
            return true;

        string sl = toString(literal, f_astContext);
        string::size_type pos = sl.find('\"');
        if ((pos != 0) && (pos != string::npos)) {
            if (sl[--pos] == 'R') {
                replacement::result res = replacement::result::found;
                string raw = toString(literal, f_astContext);
                if (getFromRaw(raw)) {
                    for (size_t i = 0; i < raw.size(); i++) {
                        string s;
                        if (escapeCharToString(raw[i], s)) {
                            raw.replace(i, 1, s);
                            i++;
                        }
                    }
                    raw.insert(0, "\"");
                    raw += "\"";
                    if (pos != 0)
                        raw = sl.substr(0, pos) + raw;
                    f_rewriter->ReplaceText(literal->getSourceRange(), raw);
                }
                f_rewriter->InsertTextBefore(literal->getLocStart(),
                                             Comment::block(replacement::info(type(), res)));
            }
        }
        return true;
    }

    bool RawStringReplacer::getFromRaw(std::string &raw) {
        string::size_type RPos = raw.find('R');
        string::size_type lParenPos = raw.find('(');
        string::size_type rParenPos = raw.find_last_of(')');
        if ((rParenPos == string::npos) || (lParenPos >= rParenPos) || (RPos >= lParenPos))
            return false;
        raw.erase(rParenPos, raw.size() - rParenPos);
        raw.erase(RPos, ++lParenPos - RPos);
        return true;
    }

    // @formatter:off
    bool RawStringReplacer::escapeCharToString(char c, string& s) { //TODO check other
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
    // @formatter:on
}
