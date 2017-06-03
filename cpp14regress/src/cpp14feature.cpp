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

#include "cpp14feature.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    void FeatureReplacer::EndSourceFileAction() {
        endSourceFileAction();
        f_rewriter->overwriteChangedFiles();
    }

    void FeatureReplacer::BeginSourceFileAction() {
        beginSourceFileAction();
    }

    FeatureReplacer::FeatureReplacer(CompilerInstance *ci) : f_compilerInstance(ci) {
        f_astContext = &(ci->getASTContext());
        f_sourceManager = &(ci->getSourceManager());
        f_langOptions = &(ci->getLangOpts());
        f_preprocessor = &(ci->getPreprocessor());
        f_rewriter = new Rewriter(*f_sourceManager, *f_langOptions);
    }
}