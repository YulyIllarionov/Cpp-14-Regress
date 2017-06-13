
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

#include "trailing_return.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;


    bool TrailingReturnSearcher::VisitFunctionDecl(clang::FunctionDecl *function) {
        if (!fromUserFile(function, f_sourceManager))
            return true;
        if (auto ft = dyn_cast_or_null<FunctionProtoType>(function->getType().getTypePtr())) {
            if (ft->hasTrailingReturn()) {
                f_rewriter->InsertTextBefore(function->getLocStart(), Comment::line(
                        replacement::info(type(), replacement::result::found)) + "\n");
            }
        }
        return true;
    }
}