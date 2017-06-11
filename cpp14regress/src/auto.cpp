
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
        replacement::result res = replacement::result::found;
        QualType deducedType = typeLoc.getTypePtr()->getDeducedType();
        if (!deducedType.isNull()) {
            if ((deducedType->isFunctionPointerType()) ||
                (deducedType->isMemberFunctionPointerType())) {
                return true;
            }
            string typeName = deducedType.getAsString(PrintingPolicy(*f_langOptions));
            //f_rewriter->ReplaceText(typeLoc.getSourceRange(), typeName);
            res = replacement::result::replaced;
            cout << typeName << " -- " << typeLoc.getEndLoc().printToString(*f_sourceManager) << endl;
        }
        //f_rewriter->InsertTextBefore(typeLoc.getLocStart(),
        //                             Comment::block(replacement::info(type(), res)));
        return true;
    }*/

    bool AutoReplacer::VisitVarDecl(VarDecl *varDecl) {
        if (!fromUserFile(varDecl, f_sourceManager))
            return true;

        if (auto at = dyn_cast<AutoType>(varDecl->getType().getTypePtr())) {
            QualType deducedType = at->getDeducedType();
            cout << deducedType->getTypeClassName()
                 << varDecl->getLocStart().printToString(*f_sourceManager) << endl;
            replacement::result res = replacement::result::found;
            SourceRange typeRange = varDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange();
            if (!deducedType.isNull()) {
                //TODO method pointer and reference -- reference not found
                //TODO field pointer and reference  -- reference not found
                //TODO array reference
                //TODO function reference
                string typeName = deducedType.getAsString(PrintingPolicy(*f_langOptions));
                if ((deducedType->isFunctionPointerType()) ||
                    (deducedType->isMemberFunctionPointerType())) {
                    string ident = (deducedType->isFunctionPointerType()) ? "(*)" : "::*";
                    size_t identOffset = (deducedType->isFunctionPointerType()) ? 2 : 3;

                    size_t pos = typeName.find(ident); //TODO change
                    if (pos != string::npos) {
                        typeName.insert(pos + identOffset, varDecl->getNameAsString());
                    } else {
                        cerr << "auto tool error" << endl;
                        return false;
                    }
                    typeRange.setEnd(Lexer::getLocForEndOfToken(varDecl->getLocation(), 0,
                                                                *f_sourceManager, *f_langOptions));
                }
                f_rewriter->ReplaceText(typeRange, typeName);
                res = replacement::result::replaced;
            }
            f_rewriter->InsertTextBefore(typeRange.getBegin(),
                                         Comment::block(replacement::info(type(), res)));
        }
        return true;
    }

    bool AutoReplacer::VisitFunctionDecl(clang::FunctionDecl *funDecl) {
        if (!fromUserFile(funDecl, f_sourceManager))
            return true;
        if (auto at = dyn_cast<AutoType>(funDecl->getReturnType().getTypePtr())) {
            QualType deducedType = at->getDeducedType();
            replacement::result res = replacement::result::found;
            if (!deducedType.isNull()) {
                f_rewriter->ReplaceText(funDecl->getReturnTypeSourceRange(), deducedType.getAsString());
                res = replacement::result::replaced;
            }
            f_rewriter->InsertTextBefore(funDecl->getReturnTypeSourceRange().getBegin(),
                                         Comment::block(replacement::info(type(), res)));
        }
        return true;
    }

}

