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

#include "base_types.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    void RangeBasedForASTConsumer::HandleTranslationUnit(ASTContext &context) {
        visitor->TraverseDecl(context.getTranslationUnitDecl());
    }

    unique_ptr <ASTConsumer> RangeBasedForFrontendAction::CreateASTConsumer(CompilerInstance &CI, StringRef file) {
        ASTConsumer *astConsumer = new RangeBasedForASTConsumer(&CI.getASTContext());
        return unique_ptr<ASTConsumer>(astConsumer);
    }
}
