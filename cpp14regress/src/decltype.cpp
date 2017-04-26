
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

    DecltypeReplacer::DecltypeReplacer(ASTContext *context, cpp14features_stat *stat, DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void DecltypeReplacer::EndFileAction() {
        for (auto i = f_rewriter->buffer_begin(), e = f_rewriter->buffer_end(); i != e; ++i) {
            const FileEntry *entry = f_context->getSourceManager().getFileEntryForID(i->first);
            string file = f_dg->getFile(entry->getName());
            std::error_code ec;
            sys::fs::remove(Twine(file));
            raw_fd_ostream rfo(StringRef(file), ec, sys::fs::OpenFlags::F_Excl | sys::fs::OpenFlags::F_RW);
            //cout << "Trying to write " << entry->getName() << " to " << file << " with " << ec.message() << endl;
            i->second.write(rfo);
        }
        //f_rewriter->overwriteChangedFiles();
    }

    bool DecltypeReplacer::VisitDeclaratorDecl(DeclaratorDecl *declaratorDecl) {
        if (!inProcessedFile(declaratorDecl, f_context))
            return true;
        if (auto dt = dyn_cast<DecltypeType>(declaratorDecl->getType().getTypePtr())) {
            QualType qt = dt->getUnderlyingType();
            string typeName = qt.getAsString();
            SourceRange typeRange;
            typeRange.setBegin(declaratorDecl->getTypeSourceInfo()->getTypeLoc().getBeginLoc());
            typeRange.setEnd(Lexer::findLocationAfterToken(
                    dt->getUnderlyingExpr()->getLocEnd(), tok::TokenKind::r_paren,
                    f_context->getSourceManager(),
                    f_context->getLangOpts(), false));
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
    }

}
