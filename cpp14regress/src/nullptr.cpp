
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

#include "nullptr.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;


    bool NullPtrReplacer::VisitCXXNullPtrLiteralExpr(CXXNullPtrLiteralExpr *nullPtrExpr) {
        if (!fromUserFile(nullPtrExpr, f_sourceManager))
            return true;
        f_rewriter->ReplaceText(SourceRange(nullPtrExpr->getLocStart(), nullPtrExpr->getLocEnd()), "0");
        f_rewriter->InsertTextBefore(nullPtrExpr->getLocStart(), Comment::block(
                replacement::info(type(), replacement::result::replaced)));
        return true;
    }

    bool NullPtrReplacer::VisitTypeLoc(clang::TypeLoc typeLoc) { //TODO inserts twice
        if (!fromUserFile(&typeLoc, f_sourceManager))
            return true;
        if(auto nullptrType = typeLoc.getTypePtr()) {
            if (nullptrType->isNullPtrType()) {
                f_rewriter->InsertTextBefore(typeLoc.getLocStart(), Comment::block(
                        replacement::info(type(),replacement::result::found)));
            }
        }
        return true;
    }

}
