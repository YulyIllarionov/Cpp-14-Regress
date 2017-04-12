
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

#include "constuctor_delegation.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    ConstructorDelegationReplacer::ConstructorDelegationReplacer(ASTContext *context, cpp14features_stat *stat)
            : f_context(context), f_stat(stat) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    bool ConstructorDelegationReplacer::VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl) {
        if (!inProcessedFile(recordDecl, f_context))
            return true;
        vector<CXXConstructorDecl *> targetCtros;
        const SourceManager &sm = f_context->getSourceManager();
        for (auto it = recordDecl->ctor_begin(); it != recordDecl->ctor_end(); it++) {
            if (it->isDelegatingConstructor()) {
                auto targetCtor = it->getTargetConstructor();
                size_t pos = find(targetCtros.begin(), targetCtros.end(), targetCtor) -
                             targetCtros.begin(); //TODO fix duplicates check
                if (pos == (targetCtros.size() - 1)) {
                    targetCtros.push_back(targetCtor);
                }
                //TODO getDefinition
                //const FunctionDecl *fDef;
                //it->isDefined(fDef);
                //const CXXConstructorDecl *ctrDef = const_cast<CXXConstructorDecl*>(fDef);


                string initFunCallParam(sm.getCharacterData((*(it->init_begin()))->getLParenLoc()),
                                        sm.getCharacterData((*(it->init_begin()))->getRParenLoc()) + 1);
                string initFunCall("\n" +
                                   initFunNameGenerator(it->getNameAsString()).toString() +
                                   initFunCallParam + ";");
                f_rewriter->InsertTextAfterToken(it->getBody()->getLocStart(), initFunCall);
                SourceRange paramRange = getParamRange(dyn_cast<FunctionDecl>(*it), f_context);
                f_rewriter->RemoveText(SourceRange(paramRange.getEnd().getLocWithOffset(1),
                                                   it->getBody()->getLocStart().getLocWithOffset(-1)));
                //SourceLocation paramEnd = getParamRange(dyn_cast<FunctionDecl>(recordDecl), f_context).getEnd();
                //SourceLocation bodyBegin = (*(it->init_rbegin()))->getRParenLoc();

                //SourceLocation paramsEnd = it->getLocation();
                //SourceLocation initEnd = (*(it->init_rbegin()))->getRParenLoc();
                //int l = sm.getCharacterData(initEnd) - sm.getCharacterData(it->getLocStart());
                //int i = 1;
                //for (; i < l; i++) {
                //    paramsEnd = Lexer::findLocationAfterToken(
                //            paramsEnd, tok::TokenKind::r_paren,
                //            f_context->getSourceManager(),
                //            f_context->getLangOpts(), false);
                //    if (paramsEnd.isValid())
                //        break;
                //    else
                //        paramsEnd = it->getLocation().getLocWithOffset(i);
                //}
                //cout << it->getLocation().printToString(sm) << " -- "
                //     << l << " - " << i << " -- " << paramsEnd.printToString(sm) << endl;
            }

            //SourceLocation paramsEnd = Lexer::findLocationAfterToken(
            //        dt->getUnderlyingExpr()->getLocEnd(), tok::TokenKind::r_paren,
            //        f_context->getSourceManager(),
            //        f_context->getLangOpts(), false);
            //
            //for (auto cit = targets.begin(); cit != targets.end(); cit++) {
            //    for (auto iit = (*cit)->init_begin(); iit != (*cit)->init_end(); iit++) {
            //        cout << std::string(
            //                f_context->getSourceManager().getCharacterData((*iit)->getSourceRange().getBegin()),
            //                f_context->getSourceManager().getCharacterData((*iit)->getSourceRange().getEnd()) -
            //                f_context->getSourceManager().getCharacterData((*iit)->getSourceRange().getBegin()))
            //             << " -- ";
            //    }
            //    cout << "------" << endl << toSting(*(cit), f_context)
            //         << endl << "------" << endl;
            //}

        }
        return true;
    }
}
