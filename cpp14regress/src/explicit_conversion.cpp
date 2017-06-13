
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

#include "explicit_conversion.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool ExplicitConversionReplacer::VisitCXXConversionDecl(clang::CXXConversionDecl *conversionDecl) {
        if (!fromUserFile(conversionDecl, f_sourceManager))
            return true;

        if (conversionDecl->isExplicitSpecified()) { //TODO change logic
            SourceLocation explBegin = conversionDecl->getLocStart();
            SourceLocation explEnd = Lexer::getLocForEndOfToken(explBegin, 0,
                                                                *f_sourceManager, *f_langOptions);
            string explStr = toString(SourceRange(explBegin, explEnd), f_astContext, false);
            replacement::result res = replacement::result::found;
            if (explStr == "explicit") {
                f_rewriter->InsertTextBefore(explBegin, Comment::block::begin());
                f_rewriter->InsertTextAfter(explEnd, Comment::block::end());
                res = replacement::result::removed;
            }
            f_rewriter->InsertTextBefore(conversionDecl->getLocStart(), Comment::line(
                    replacement::info(type(), res)) + "\n");
        }
        return true;
    }
}