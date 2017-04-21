
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

    ImplictSizeofReplacer::ImplictSizeofReplacer(ASTContext *context, cpp14features_stat *stat, DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void ImplictSizeofReplacer::EndFileAction() {
        for (auto i = f_rewriter->buffer_begin(), e = f_rewriter->buffer_end(); i != e; ++i) {
            const FileEntry *entry = f_context->getSourceManager().getFileEntryForID(i->first);
            string file = f_dg->getFile(entry->getName());
            std::error_code ec;
            sys::fs::remove(Twine(file));
            raw_fd_ostream rfo(StringRef(file), ec, sys::fs::OpenFlags::F_Excl | sys::fs::OpenFlags::F_RW);
            //cout << "Trying to write " << entry->getName() << " to " << file << " with " << ec.message() << endl;
            i->second.write(rfo);
        }
    }

    bool ImplictSizeofReplacer::VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *sizeofOrAlignof) {
        if (!inProcessedFile(sizeofOrAlignof, f_context))
            return true;
        if (sizeofOrAlignof->getKind() == UnaryExprOrTypeTrait::UETT_SizeOf) {
            if (!sizeofOrAlignof->isArgumentType()) {
                Expr *arg = sizeofOrAlignof->getArgumentExpr();
                int parens = 0;
                while (auto parenExpr = dyn_cast<ParenExpr>(arg)) {
                    parens++;
                    arg = parenExpr->getSubExpr();
                }
                if (auto dre = dyn_cast_or_null<DeclRefExpr>(arg)) {
                    if (auto fd = dyn_cast_or_null<FieldDecl>(dre->getDecl())) {
                        string type = fd->getType().getAsString();
                        if (parens == 0) {
                            type.insert(0, 1, '(');
                            type.push_back(')');
                        }
                        f_rewriter->ReplaceText(dre->getSourceRange(), type);
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
        return true;
    }
}
