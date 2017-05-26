
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

#include "delete.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;


    bool DeleteReplacer::VisitCXXMethodDecl(clang::CXXMethodDecl *methodDecl) {
        if (!fromUserFile(methodDecl, f_sourceManager))
            return true;
        if (methodDecl->isDeleted()) {
            SourceLocation b = Lexer::getLocForEndOfToken(getParamRange(methodDecl, f_astContext).getEnd(), 0,
                                                          *f_sourceManager, *f_langOptions);
            SourceLocation e = methodDecl->getLocEnd();
            string access;
            switch (methodDecl->getAccess()) {
                case AccessSpecifier::AS_protected : {
                    access = "\nprotected:\n";
                    break;
                }
                case AccessSpecifier::AS_public : {
                    access = "\npublic:\n";
                    break;
                }
                default :
                    break;
            }
            f_rewriter->ReplaceText(SourceRange(b, e), " {}");
            if (methodDecl->getAccess() != AccessSpecifier::AS_private)
                f_rewriter->InsertTextBefore(methodDecl->getLocStart(), "private:\n");
            f_rewriter->InsertTextBefore(methodDecl->getLocStart(), Comment::line(
                    replacement::begin(type(), replacement::result::replaced)) + "\n");
            //TODO doesn't work
            SourceLocation accessLoc = Lexer::getLocForEndOfToken(e, 0, *f_sourceManager, *f_langOptions);
            if (methodDecl->getAccess() != AccessSpecifier::AS_private)
                f_rewriter->InsertTextAfterToken(accessLoc, access);
            f_rewriter->InsertTextAfterToken(accessLoc, Comment::line(
                    replacement::end(type(), replacement::result::replaced)) + "\n");
        }
        return true;
    }


}