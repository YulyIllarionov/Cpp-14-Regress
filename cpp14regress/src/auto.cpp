
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

#include "auto.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    /*bool AutoReplacer::VisitAutoTypeLoc(clang::AutoTypeLoc typeLoc) {
        if (!fromUserFile(&typeLoc, f_sourceManager))
            return true;
        cout << "Auto visited" << endl;
        QualType deducedType = typeLoc.getTypePtr()->getDeducedType();
        if (!deducedType.isNull()) {
            cout << "Auto deduced" << endl;
            QualType pointeeType = deducedType;
            if (deducedType->isReferenceType() || deducedType->isPointerType())
                pointeeType = deducedType->getPointeeType();
            if (auto elaboratedType = dyn_cast<ElaboratedType>(pointeeType))
                pointeeType = elaboratedType->getNamedType();

            //cout << "Type name: " << pointeeType->getTypeClassName() << " -- "
            //     << typeLoc.getLocStart().printToString(*f_sourceManager) << endl;
            replacement::result res = replacement::result::replaced;
            string reason;
            if (pointeeType->isFunctionType()) {
                res = replacement::result::found;
                reason = " is function pointer";
            }
            if (deducedType->isDependentType()) {
                res = replacement::result::found;
                reason = " is dependent type";
            }
            if (auto recordDecl = pointeeType->getAsCXXRecordDecl())
                if (recordDecl->isLambda()) {
                    res = replacement::result::found;
                    reason = " is lambda";
                }
            if (auto tagDecl = pointeeType->getAsTagDecl())
                if (tagDecl->getNameAsString().empty()) {
                    res = replacement::result::found;
                    reason = " is anonymous";
                }
            if (isa<DecltypeType>(deducedType)) {
                res = replacement::result::found;
                reason = " is still decltype";
            }

            if (res != replacement::result::found) {
                if (typeLoc.getSourceRange().isValid()) {
                    PrintingPolicy pp(*f_langOptions);
                    //pp.SuppressUnwrittenScope = true;
                    f_rewriter->InsertTextAfterToken(typeLoc.getLocEnd(), Comment::block::end());
                    f_rewriter->InsertTextAfterToken(typeLoc.getLocEnd(),
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
    }*/

    bool AutoReplacer::VisitVarDecl(VarDecl *varDecl) {
        if (!fromUserFile(varDecl, f_sourceManager))
            return true;
        if (!f_firstInMultiple) {
            f_multipleDecl.erase(varDecl);
            //cout << "Pop from set: " << varDecl << endl;
            if (f_multipleDecl.empty())
                f_firstInMultiple = true;
            return true;
        }
        if (f_multipleDecl.find(varDecl) != f_multipleDecl.end()) {
            f_multipleDecl.erase(varDecl);
            //cout << "First pop from set: " << varDecl << endl;
            f_firstInMultiple = false;
        }


        /*cout << varDecl->getNameAsString() << " -- ";
        const auto &parents = f_astContext->getParents(*varDecl);
        for (auto parent : parents) {
            if (auto stmt = parent.get<Stmt>()) {
                cout << "Stmt: " << stmt->getStmtClassName();
            } else if (auto decl = parent.get<Decl>()) {
                cout << "Decl: " << decl->getDeclKindName();
            }
            cout << endl;
        }*/

        QualType qt = varDecl->getType();
        if (qt.isNull())
            return true;
        if (auto at = dyn_cast_or_null<AutoType>(qt.getTypePtr())) {
            replacement::result res = replacement::result::found;
            SourceRange typeRange = varDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange();
            QualType deducedType = at->getDeducedType();
            string reason;
            if (typeCanBeReplaced(deducedType, reason)) {
                //cout << "Auto type: " << deducedType->getTypeClassName() << " -- "
                //     << varDecl->getLocStart().printToString(*f_sourceManager) << endl;
                //TODO array reference
                //TODO function reference
                string typeName = deducedType.getAsString(PrintingPolicy(*f_langOptions));
                QualType pointerType = deducedType;
                size_t ptrNumber = 0;
                while (pointerType->isReferenceType() || pointerType->isPointerType()) {
                    ptrNumber++;
                    pointerType = pointerType->getPointeeType();
                    //cout << string("Type " + to_string(ptrNumber)) << pointerType->getTypeClassName() << endl;
                }
                //cout << deducedType->getTypeClassName() << endl;

                if (at->isDecltypeAuto()) {
                    typeRange.setEnd(findTokenBeginAfterLoc(typeRange.getEnd(), tok::TokenKind::r_paren,
                                                            1, f_astContext));
                }
                if ((pointerType->isFunctionPointerType()) ||
                    (pointerType->isMemberFunctionPointerType())) {
                    string ident = (pointerType->isFunctionPointerType()) ?
                                   string("(*" + string(ptrNumber, '*') + ")") :
                                   string("::*" + string(ptrNumber, '*')); //TODO check
                    size_t identOffset = (pointerType->isFunctionPointerType())
                                         ? ident.size() - 1 : ident.size();
                    size_t pos = typeName.find(ident); //TODO change
                    //cout << "Identifier: " << ident << " -- " << typeName.substr(0, pos + identOffset) << endl;
                    if (pos != string::npos) {
                        typeName.insert(pos + identOffset, varDecl->getNameAsString());
                    } else {
                        typeName = replacement::info(type(), replacement::result::error);
                    }
                    typeRange.setEnd(Lexer::getLocForEndOfToken(varDecl->getLocation(), 0,
                                                                *f_sourceManager, *f_langOptions));
                }
                if (typeRange.isValid()) {
                    f_rewriter->ReplaceText(typeRange, typeName);
                    res = replacement::result::replaced;
                }
            }
            f_rewriter->InsertTextBefore(typeRange.getBegin(),
                                         Comment::block(replacement::info(type(), res) + reason));
        }
        return true;
    }

    bool AutoReplacer::VisitFunctionDecl(clang::FunctionDecl *funDecl) {
        if (!fromUserFile(funDecl, f_sourceManager))
            return true;
        QualType qt = funDecl->getReturnType();
        if (qt.isNull())
            return true;
        if (auto at = dyn_cast_or_null<AutoType>(qt.getTypePtr())) {
            replacement::result res = replacement::result::found;
            string reason;
            QualType deducedType = at->getDeducedType();
            SourceRange typeRange = funDecl->getReturnTypeSourceRange();
            if (typeCanBeSimplyReplaced(deducedType, reason)) {
                if (at->isDecltypeAuto()) {
                    typeRange.setEnd(findTokenBeginAfterLoc(typeRange.getEnd(), tok::TokenKind::r_paren,
                                                            1, f_astContext));
                }
                if (typeRange.isValid()) {
                    f_rewriter->ReplaceText(typeRange,
                                            deducedType.getAsString(PrintingPolicy(*f_langOptions)));
                    res = replacement::result::replaced;
                }
            }
            f_rewriter->InsertTextBefore(typeRange.getBegin(),
                                         Comment::block(replacement::info(type(), res) + reason));
        }

        return true;
    }

    bool AutoReplacer::VisitDeclStmt(clang::DeclStmt *declStmt) {
        for (auto decl : declStmt->decls()) {
            if (isa<VarDecl>(decl)) {
                //cout << "Push to set: " << decl << endl;
                f_multipleDecl.insert(decl);
            }
        }
        return true;
    }

}

