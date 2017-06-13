
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

#include "override.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;


    bool OverrideReplacer::VisitCXXMethodDecl(CXXMethodDecl *methodDecl) {
        if (!fromUserFile(methodDecl, f_sourceManager))
            return true;

        auto pos = find_if(methodDecl->attr_begin(), methodDecl->attr_end(), [](Attr *a) {
            return (a->getKind() == attr::Kind::Override);
        });
        if (pos != methodDecl->attr_end()) {
            f_rewriter->InsertTextAfterToken((*pos)->getRange().getEnd(), Comment::block::end());
            f_rewriter->InsertTextBefore((*pos)->getRange().getBegin(), Comment::block::begin());
            f_rewriter->InsertTextBefore(methodDecl->getLocStart(), Comment::line(
                    replacement::info(type(),replacement::result::removed)) + "\n");
        }
        return true;
    }
}