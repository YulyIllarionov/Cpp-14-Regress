
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

#include "implict_sizeof.h"
#include "utils.h"

#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool
    MemberSizeofReplacer::VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *sizeofOrAlignof) {
        if (!fromUserFile(sizeofOrAlignof, f_sourceManager))
            return true;

        if (sizeofOrAlignof->getKind() == UnaryExprOrTypeTrait::UETT_SizeOf) {
            if (!sizeofOrAlignof->isArgumentType()) {
                Expr *arg = sizeofOrAlignof->getArgumentExpr();
                if (arg) {
                    SourceRange replaceRange = arg->getSourceRange();
                    //TODO strange behaviour
                    replaceRange.setEnd(Lexer::getLocForEndOfToken(replaceRange.getEnd(), 0,
                                                                   *f_sourceManager, *f_langOptions));
                    int parentheses = 0;
                    while (auto parenExpr = dyn_cast<ParenExpr>(arg)) {
                        parentheses++;
                        arg = parenExpr->getSubExpr();
                    }
                    if (auto dre = dyn_cast_or_null<DeclRefExpr>(arg)) {
                        if (auto fd = dyn_cast_or_null<FieldDecl>(dre->getDecl())) {
                            if (replaceRange.isValid()) {
                                string typeName = fd->getType().getAsString(PrintingPolicy(*f_langOptions));
                                for (int i = 0; i < max(parentheses, 1); i++) {
                                    typeName.insert(0, 1, '(');
                                    typeName.push_back(')');
                                }
                                string info = replacement::info(type(), replacement::result::replaced);
                                info += " from ";
                                f_rewriter->InsertTextBefore(replaceRange.getBegin(), info);
                                f_rewriter->InsertTextBefore(replaceRange.getBegin(),
                                                             Comment::block::begin());
                                f_rewriter->InsertTextAfter(replaceRange.getEnd(),
                                                            Comment::block::end());
                                f_rewriter->InsertTextBefore(replaceRange.getBegin(), typeName);
                            } else {
                                f_rewriter->InsertTextBefore(dre->getLocStart(), Comment::block(
                                        replacement::info(type(), replacement::result::found)));
                            }
                        }
                        //if (dre->hasQualifier()) { //TODO check another SpecifierKind
                        //    if ((dre->getQualifier()->getKind() ==
                        //         clang::NestedNameSpecifier::SpecifierKind::TypeSpec) ||
                        //        (dre->getQualifier()->getKind() ==
                        //         clang::NestedNameSpecifier::SpecifierKind::TypeSpecWithTemplate)) {
                        //         here
                        //    }
                        //}
                    }

                }
            }
        }
        return true;
    }
}
