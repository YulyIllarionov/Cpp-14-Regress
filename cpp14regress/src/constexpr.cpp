
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

#include "constexpr.h"
#include "utils.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;


    bool ConstexprSearcher::VisitFunctionDecl(clang::FunctionDecl *functionDecl) {
        if (!fromUserFile(functionDecl, f_sourceManager))
            return true;
        if (functionDecl->isConstexpr())
            f_rewriter->InsertTextBefore(functionDecl->getLocStart(), Comment::line(
                    replacement::info(type(), replacement::result::found)) + "\n");
        return true;
    }

    bool ConstexprSearcher::VisitVarDecl(clang::VarDecl *varDecl) {
        if (!fromUserFile(varDecl, f_sourceManager))
            return true;
        if (varDecl->isConstexpr())
            f_rewriter->InsertTextBefore(varDecl->getLocStart(), Comment::line(
                    replacement::info(type(), replacement::result::found)) + "\n");
        return true;
    }
}