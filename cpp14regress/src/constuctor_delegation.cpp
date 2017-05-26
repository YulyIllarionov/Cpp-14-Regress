
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

    string ConstructorDelegationReplacer::initFunName(const CXXConstructorDecl *ctr) {
        return (ctr->getNameAsString() + f_seed);
    }

    string ConstructorDelegationReplacer::initFunCall(const CXXConstructorDecl *delegating) {
        string call;
        const SourceManager &sm = f_astContext->getSourceManager();
        auto init = find_if(delegating->init_begin(), delegating->init_end(),
                            [](CXXCtorInitializer *init) -> bool {
                                if (init->isDelegatingInitializer())
                                    return true;
                                return false;
                            });
        if (init == delegating->init_end())
            return string();
        SourceLocation b((*init)->getLParenLoc());
        SourceLocation e((*init)->getRParenLoc().getLocWithOffset(1));
        string params(sm.getCharacterData(b), sm.getCharacterData(e));
        return string(initFunName(delegating) + params + ";");
    }

    bool ConstructorDelegationReplacer::VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl) {
        if (!fromUserFile(recordDecl, f_sourceManager))
            return true;

        if (recordDecl->ctor_begin() == recordDecl->ctor_end())
            return true;
        vector<CXXConstructorDecl *> targetCtors;
        //Delegation replacement
        for (auto declaration = recordDecl->ctor_begin();
             declaration != recordDecl->ctor_end(); declaration++) {

            const FunctionDecl *fd = nullptr;
            declaration->hasBody(fd);
            const CXXConstructorDecl *definition = dyn_cast_or_null<CXXConstructorDecl>(fd);

            if (definition) {
                if (definition->isDelegatingConstructor()) {
                    replacement::result res = replacement::result::found;

                    auto targetCtor = definition->getTargetConstructor();
                    auto pos = find(targetCtors.begin(), targetCtors.end(), //TODO move
                                    targetCtor);
                    if (pos == targetCtors.end()) {
                        targetCtors.push_back(targetCtor);
                    }

                    SourceRange delegRange;
                    delegRange.setBegin(findTokenBeginAfterLoc(definition->getLocation(),
                                                               tok::TokenKind::colon, 1, f_astContext));
                    delegRange.setEnd(definition->getBody()->getLocStart().getLocWithOffset(-1)); //TODO
                    if (delegRange.isInvalid())
                        continue;

                    const FunctionDecl *fd1 = nullptr;
                    targetCtor->hasBody(fd1);
                    const CXXConstructorDecl *targetCtorDef = dyn_cast_or_null<CXXConstructorDecl>(fd1);
                    if (targetCtorDef) {
                        if (find_if(targetCtorDef->init_begin(), targetCtorDef->init_end(),
                                    [](CXXCtorInitializer *init) -> bool {
                                        if (init->isWritten() && !init->isDelegatingInitializer())
                                            return true;
                                        return false;
                                    }) != targetCtorDef->init_end()) {
                            SourceRange targetInitRange;
                            //TODO передача исходного параметра


                            targetInitRange.setBegin(findTokenBeginAfterLoc(targetCtor->getLocation(),
                                                                            tok::TokenKind::colon,
                                                                            1, f_astContext));
                            targetInitRange.setEnd(targetCtor->getBody()->getLocStart());
                            if (targetInitRange.isValid()) {
                                string targetInits = toString(targetInitRange, f_astContext, false);
                                f_rewriter->ReplaceText(delegRange, targetInits);
                                res = replacement::result::replaced;
                            }
                        } else {
                            f_rewriter->RemoveText(delegRange);
                            res = replacement::result::removed;
                        }
                    }
                    f_rewriter->InsertTextBefore(definition->getLocStart(), Comment::line(
                            replacement::info(type(), res)) + "\n");

                    SourceLocation bodyStart = Lexer::getLocForEndOfToken(
                            definition->getBody()->getLocStart(), 0, *f_sourceManager, *f_langOptions);
                    string initCall = initFunCall(definition);
                    if (initCall.empty()) {
                        initCall = string(initFunName(definition) + "(" + Comment::block(
                                replacement::info(type(), replacement::result::error)) + ");");
                    }
                    string call("\n" + initCall);
                    f_rewriter->InsertText(bodyStart, call, true, true);
                }
            }
        }
        //Init functions creation
        if (!targetCtors.empty()) {
            f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), Comment::line(
                    replacement::end(type(), replacement::result::inserted)) + "\n");
        }
        for (auto declaration = targetCtors.rbegin();
             declaration != targetCtors.rend(); declaration++) {
            const FunctionDecl *fd;
            (*declaration)->isDefined(fd);
            const CXXConstructorDecl *definition = dyn_cast_or_null<CXXConstructorDecl>(fd);

            if (definition) {
                SourceRange paramRange;
                paramRange.setBegin(findTokenBeginAfterLoc(definition->getLocation(),
                                                           tok::TokenKind::l_paren, 1, f_astContext));
                paramRange.setEnd(findTokenEndAfterLoc(definition->getLocation(),
                                                       tok::TokenKind::r_paren, f_astContext));
                string initFunParams;
                if (paramRange.isValid())
                    initFunParams = toString(paramRange, f_astContext, false);
                else
                    initFunParams = string("(" + replacement::info(type(), replacement::result::error) + ")");
                string initFunBody = "{\n";
                string delegation = initFunCall(definition);
                if (!delegation.empty()) {
                    initFunBody += delegation;
                    initFunBody += "\n";
                }
                SourceLocation bodyStart = Lexer::getLocForEndOfToken(
                        definition->getBody()->getLocStart(), 0, *f_sourceManager, *f_langOptions);
                initFunBody += toString(SourceRange(bodyStart, definition->getBodyRBrace()), f_astContext);
                string initFunDefinition(" \nvoid " + initFunName(definition) +
                                         initFunParams + " " + initFunBody + "\n");
                f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), initFunDefinition);
            }
        }
        if (!targetCtors.empty()) {
            f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), "private:\n");
            f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), Comment::line(
                    replacement::begin(type(), replacement::result::inserted)) + "\n");
        }
        return true;
    }
}
