
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

#include "alias_type.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool AliasTypeReplacer::VisitTypeAliasDecl(clang::TypeAliasDecl *aliasTypeDecl) {
        if (!fromUserFile(aliasTypeDecl, f_sourceManager))
            return true;

        if (!(aliasTypeDecl->getDescribedAliasTemplate())) {
            string oldTypedef = "typedef ";
            QualType qt = aliasTypeDecl->getUnderlyingType();
            replacement::result res = replacement::result::found;
            if (!qt->isDependentType()) { //TODO
                string synonym = aliasTypeDecl->getNameAsString();
                if ((qt->isFunctionPointerType()) ||
                    (qt->isMemberFunctionPointerType())) {
                    string ident = (qt->isFunctionPointerType()) ? "(*)" : "::*";
                    size_t identOffset = (qt->isFunctionPointerType()) ? 2 : 3;
                    string typeName = qt.getAsString(PrintingPolicy(*f_langOptions));
                    string::size_type pos = typeName.find(ident); //TODO change
                    if (pos == string::npos) {
                        f_rewriter->InsertTextBefore(aliasTypeDecl->getLocStart(), Comment::line(
                                replacement::info(type(), res)) + "\n");
                        return true;
                    }
                    typeName.insert(pos + identOffset, synonym);
                    oldTypedef += typeName;
                } else {
                    //oldTypedef += toString(aliasTypeDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange(),
                    //                       f_astContext);
                    oldTypedef += qt.getAsString(PrintingPolicy(*f_langOptions));
                    oldTypedef += " ";
                    oldTypedef += synonym;
                }
                //SourceRange sr = aliasTypeDecl->getSourceRange();
                //sr.setEnd(Lexer::getLocForEndOfToken(sr.getEnd(), 0, *f_sourceManager, *f_langOptions));
                f_rewriter->ReplaceText(aliasTypeDecl->getSourceRange(), oldTypedef);
                res = replacement::result::replaced;
            }
            f_rewriter->InsertTextBefore(aliasTypeDecl->getLocStart(), Comment::line(
                    replacement::info(type(), res)) + "\n");
        }
        return true;
    }
}