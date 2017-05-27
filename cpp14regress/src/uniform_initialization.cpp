
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

#include "uniform_initialization.h"
#include "utils.h"
#include <vector>
#include "ast_to_dot.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool UniformInitReplacer::VisitCXXConstructExpr(CXXConstructExpr *constructExpr) {
        if (!fromUserFile(constructExpr, f_sourceManager))
            return true;
        if (constructExpr->isListInitialization() &&
            !constructExpr->isStdInitListInitialization()) {
            if (const auto constructDecl = constructExpr->getConstructor()) {
                replacement::result res = replacement::result::replaced;
                SourceRange braceRange = constructExpr->getParenOrBraceRange();
                if (constructExpr->getLocStart() == braceRange.getBegin()) {
                    string ctorName = constructDecl->getNameAsString();
                    if (auto inst = dyn_cast_or_null<ClassTemplateSpecializationDecl>(
                            constructDecl->getParent())) {
                        QualType ctorType = f_astContext->getRecordType(constructDecl->getParent());
                        if (!ctorType.isNull()) {
                            ctorName = ctorType.getAsString(PrintingPolicy(*f_langOptions));
                        } else {
                            res = replacement::result::unsureReplaced;
                            string ctorTemplArgs;
                            const TemplateArgumentList &tal = inst->getTemplateInstantiationArgs();
                            for (unsigned i = 0; i < tal.size();) {
                                QualType qt = tal[i].getAsType();
                                if (!qt.isNull()) {
                                    ctorTemplArgs += qt.getAsString(PrintingPolicy(*f_langOptions));
                                } else if (auto nonType = tal[i].getAsDecl()) {
                                    ctorTemplArgs += nonType->getNameAsString();
                                } else {
                                    ctorTemplArgs += Comment::block(
                                            replacement::info(type(), replacement::result::error) +
                                            " in template arg");
                                }
                                if (++i != tal.size())
                                    ctorTemplArgs += ", ";
                            }
                            if (!ctorTemplArgs.empty()) {
                                ctorName += string("<" + ctorTemplArgs + ">");
                            }
                        }
                        f_rewriter->InsertTextBefore(braceRange.getBegin(), ctorName);
                    }
                    f_rewriter->InsertTextBefore(braceRange.getBegin(),
                                                 Comment::block(replacement::info(type(), res)));
                    f_rewriter->ReplaceText(braceRange.getBegin(), 1, "(");
                    f_rewriter->ReplaceText(braceRange.getEnd(), 1, ")");
                }
            }
        }
        return true;
    }
}
