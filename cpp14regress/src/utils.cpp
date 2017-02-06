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

#include "utils.h"

namespace cpp14regress{

    using namespace std;
    using namespace clang;

    bool  RecursiveVariableReplacer::VisitDeclRefExpr(DeclRefExpr *dre)
    {
        if (dre->getDecl()->getNameAsString() == f_variable->getNameAsString()) {
            f_rewriter->ReplaceText(SourceRange(dre->getLocStart(),
                                              dre->getLocEnd()), f_generator->toString());
        }
        return true;
    }

    template<class CoreClass>
    string stringFromSource(CoreClass *cc, ASTContext* context)
    {
        const SourceManager &sm = context->getSourceManager();
        const LangOptions &lo = context->getLangOpts();
        SourceLocation b(cc->getLocStart()), _e(cc->getLocEnd());
        SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, lo));
        return string(sm.getCharacterData(b),
                      sm.getCharacterData(e)-sm.getCharacterData(b));
    }

}