
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

#include "digit_separators.h"
#include "utils.h"

#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;


    bool DigitSeparatorReplacer::VisitIntegerLiteral(IntegerLiteral *literal) {
        if (!fromUserFile(literal, f_sourceManager))
            return true;
        string i = toString(literal, f_astContext);
        if (removeSeparators(i)) {
            f_rewriter->ReplaceText(literal->getSourceRange(), i);
            f_rewriter->InsertTextAfterToken(literal->getLocEnd(), Comment::block(
                    replacement::info(type(), replacement::result::removed)));
        }
        return true;
    }

    bool DigitSeparatorReplacer::VisitFloatingLiteral(clang::FloatingLiteral *literal) {
        if (!fromUserFile(literal, f_sourceManager))
            return true;
        string f = toString(literal, f_astContext);
        if (removeSeparators(f)) {
            f_rewriter->ReplaceText(literal->getSourceRange(), f);
            f_rewriter->InsertTextAfterToken(literal->getLocEnd(), Comment::block(
                    replacement::info(type(), replacement::result::removed)));
        }
        return true;
    }

    bool DigitSeparatorReplacer::removeSeparators(std::string &literal) {
        bool found = false;
        string::size_type pos = literal.find('\'');
        while (pos != string::npos) {
            literal.erase(pos, 1);
            pos = literal.find('\'', pos);
            if (!found) found = true;
        }
        return found;
    }

}
