
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
#include "clang/Lex/LiteralSupport.h"
#include "llvm/ADT/DenseMap.h"

#include "binary_literals.h"
#include "utils.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool BinaryLiteralReplacer::VisitIntegerLiteral(clang::IntegerLiteral *intLiteral) {
        if (!fromUserFile(intLiteral, f_sourceManager))
            return true;
        /*const char *litBegin = f_sourceManager->getCharacterData(intLiteral->getLocStart());
        SourceLocation litEndLoc = Lexer::getLocForEndOfToken(intLiteral->getLocEnd(), 0,
                                                              *f_sourceManager, *f_langOptions);
        unsigned literalSize = f_sourceManager->getCharacterData(litEndLoc) - litBegin;
        StringRef litStrRef(litBegin, literalSize);
        NumericLiteralParser nlp(litStrRef, intLiteral->getLocStart(), *f_preprocessor);
        if (nlp.getRadix() == 2)*/
        string literalStr = toString(intLiteral, f_astContext);
        if ((literalStr.find("0b") != string::npos) || (literalStr.find("0B") != string::npos)) {
            string hex("0x" + intLiteral->getValue().toString(16, false));
            f_rewriter->InsertTextAfterToken(intLiteral->getLocEnd(), Comment::block(
                    replacement::info(type(), replacement::result::replaced)));
            f_rewriter->ReplaceText(intLiteral->getSourceRange(), hex);
        }
        return true;
    }
}