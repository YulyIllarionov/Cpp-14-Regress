
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

#include "decltype.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool DecltypeReplacer::VisitDecltypeTypeLoc(clang::DecltypeTypeLoc typeLoc) {
        if (!fromUserFile(&typeLoc, f_sourceManager))
            return true;
        QualType deducedType = typeLoc.getTypePtr()->getUnderlyingType();
        if (!deducedType.isNull()) {

            string reason;
            replacement::result res = (typeCanBeSimplyReplaced(deducedType, reason)) ?
                                      replacement::result::replaced : replacement::result::found;
            if (res != replacement::result::found) {
                SourceLocation decltypeBegin = typeLoc.getLocStart();
                SourceLocation decltypeEnd = typeLoc.getTypePtr()->getUnderlyingExpr()->getLocEnd();
                decltypeEnd = findTokenBeginAfterLoc(decltypeEnd, tok::TokenKind::r_paren,
                                                     1, f_astContext); //TODO fix
                SourceRange decltypeRange(decltypeBegin, decltypeEnd);
                if (decltypeRange.isValid()) {
                    PrintingPolicy pp(*f_langOptions);
                    //pp.SuppressUnwrittenScope = true;
                    f_rewriter->InsertTextAfterToken(decltypeRange.getEnd(), Comment::block::end());
                    f_rewriter->InsertTextAfterToken(decltypeRange.getEnd(),
                                                     deducedType.getAsString(pp));
                } else {
                    f_rewriter->InsertTextBefore(typeLoc.getLocStart(), Comment::block::end());
                    res = replacement::result::found;
                }
            } else {
                f_rewriter->InsertTextBefore(typeLoc.getLocStart(), Comment::block::end());
            }
            f_rewriter->InsertTextBefore(typeLoc.getLocStart(),
                                         Comment::block::begin() + replacement::info(type(), res) + reason + " ");
        }
        return true;
    }

    /*bool DecltypeReplacer::VisitTypeLoc(clang::TypeLoc typeLoc) {
        if (!fromUserFile(&typeLoc, f_sourceManager))
            return true;
        if (auto at = dyn_cast<DecltypeType>(typeLoc.getTypePtr())) {
            cout << toString(&typeLoc, f_astContext);
            replacement::result res = replacement::result::found;
            QualType deducedType = at->getUnderlyingType();
            if (!deducedType.isNull()) {
                string typeName = deducedType.getAsString(PrintingPolicy(*f_langOptions));
                cout << " -2- " << typeName;
                f_rewriter->ReplaceText(typeLoc.getSourceRange(), typeName);
                res = replacement::result::replaced;
            }
            cout << endl;
            f_rewriter->InsertTextBefore(typeLoc.getLocStart(),
                                         Comment::block(replacement::info(type(), res)));
        }
        return true;
    }*/

    /*bool DecltypeReplacer::VisitDeclaratorDecl(DeclaratorDecl *declaratorDecl) {
        if (!fromUserFile(declaratorDecl, f_sourceManager))
            return true;

        if (auto dt = dyn_cast<DecltypeType>(declaratorDecl->getType().getTypePtr())) {
            QualType qt = dt->getUnderlyingType();
            string typeName = qt.getAsString();
            SourceRange typeRange;
            typeRange.setBegin(declaratorDecl->getTypeSourceInfo()->getTypeLoc().getBeginLoc());
            typeRange.setEnd(Lexer::findLocationAfterToken(
                    dt->getUnderlyingExpr()->getLocEnd(), tok::TokenKind::r_paren,
                    *,
                    f_astContext->getLangOpts(), false));
            if (qt->isFunctionPointerType()) { //TODO decltype() a, b, c; ???
                size_t pointerPos = typeName.find("(*)"); //TODO change
                if (pointerPos != string::npos) {
                    typeName.insert(pointerPos + 2, declaratorDecl->getNameAsString());
                } else {
                    cerr << "decltype tool error" << endl;
                    return false;
                }
                typeRange.setEnd(Lexer::getLocForEndOfToken(declaratorDecl->getLocation(), 0,
                                                            f_context->getSourceManager(),
                                                            f_context->getLangOpts()));
            }
            f_rewriter->ReplaceText(typeRange, typeName);
            //if (!qt.isNull()) {
            //}
        }
        return true;
    }*/

}
