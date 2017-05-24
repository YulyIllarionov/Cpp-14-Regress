
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

    bool AutoReplacer::VisitTypeLoc(clang::TypeLoc typeLoc) {
        if (!fromUserFile(&typeLoc, f_sourceManager))
            return true;
        if (auto at = dyn_cast<AutoType>(typeLoc.getTypePtr())) {
            replacement::result res = replacement::result::found;
            QualType deducedType = at->getDeducedType();
            if (!deducedType.isNull()) {
                cout << "deduced" << endl;
                if ((deducedType->isFunctionPointerType()) ||
                    (deducedType->isMemberFunctionPointerType())) {
                    return true;
                }
                string typeName = deducedType.getAsString(PrintingPolicy(*f_langOptions));
                f_rewriter->ReplaceText(typeLoc.getSourceRange(), typeName);
                res = replacement::result::replaced;
            }
            f_rewriter->InsertTextBefore(typeLoc.getLocStart(),
                                         Comment::block(replacement::info(type(), res)));
        }
        return true;
    }

    bool AutoReplacer::VisitVarDecl(clang::VarDecl *varDecl) {
        if (!fromUserFile(varDecl, f_sourceManager))
            return true;
        if (auto at = dyn_cast<AutoType>(varDecl->getType().getTypePtr())) {
            replacement::result res = replacement::result::found;
            QualType deducedType = at->getDeducedType();
            if (!deducedType.isNull()) {
                string typeName = deducedType.getAsString(PrintingPolicy(*f_langOptions));
                SourceRange typeRange = varDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange();
                if ((deducedType->isFunctionPointerType()) ||
                    (deducedType->isMemberFunctionPointerType())) {
                    string ident = (deducedType->isFunctionPointerType()) ? "(*)" : "::*";
                    size_t identOffset = (deducedType->isFunctionPointerType()) ? 2 : 3;

                    size_t pos = typeName.find(ident); //TODO change
                    if (pos != string::npos) {
                        typeName.insert(pos + identOffset, varDecl->getNameAsString());
                        typeRange.setEnd(Lexer::getLocForEndOfToken(varDecl->getLocation(), 0,
                                                                    *f_sourceManager,
                                                                    *f_langOptions));
                        f_rewriter->ReplaceText(typeRange, typeName);
                        res = replacement::result::replaced;
                    }
                    f_rewriter->InsertTextBefore(typeRange.getBegin(),
                                                 Comment::block(replacement::info(type(), res)));
                }
            }
        }
        return true;
    }

    /*bool AutoReplacer::VisitVarDecl(VarDecl *varDecl) {
        if (!inProcessedFile(varDecl, f_context))
            return true;
        if (auto at = dyn_cast<AutoType>(varDecl->getType().getTypePtr())) {
            //TODO need GNUAutoType?
            QualType deducedType = at->getDeducedType(); //TODO understand deduced
            if (!deducedType.isNull()) {

                cout << toString(varDecl, f_context) << " -- " << deducedType.getAsString()
                     << " -- " << varDecl->getDeclKindName() << " -- "
                     << deducedType->getTypeClassName() << " -- "
                     << varDecl->getLocation().printToString(f_context->getSourceManager())
                     << endl << console_hline('-') << endl;

                //TODO method pointer and reference -- reference not found
                //TODO field pointer and reference  -- reference not found
                //TODO array reference
                //TODO function reference
                PrintingPolicy pp(f_context->getLangOpts());
                string typeName = deducedType.getAsString(pp);
                SourceRange typeRange = varDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange();
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
                                                                f_context->getSourceManager(),
                                                                f_context->getLangOpts()));
                }
                f_rewriter->ReplaceText(typeRange, typeName);
            } else {
                //TODO not deduced
            }
        }
        return true;
    }

    bool AutoReplacer::VisitFunctionDecl(clang::FunctionDecl *fun) {
        if (!inProcessedFile(fun, f_context))
            return true;
        if (auto at = dyn_cast<AutoType>(fun->getReturnType().getTypePtr())) {
            QualType deducedType = at->getDeducedType(); //TODO understand deduced
            if (!deducedType.isNull()) {
                f_rewriter->ReplaceText(fun->getReturnTypeSourceRange(), deducedType.getAsString());

                //cout << "Auto function" << toString(fun, f_context) << " -- "
                //     << fun->getLocation().printToString(f_context->getSourceManager()) << endl;
            } else {
                //TODO not deduced
            }
        }
        return true;
    }*/
}

