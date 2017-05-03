
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

    AutoReplacer::AutoReplacer(ASTContext *context, cpp14features_stat *stat,
                               DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void AutoReplacer::EndFileAction() {
        for (auto i = f_rewriter->buffer_begin(), e = f_rewriter->buffer_end(); i != e; ++i) {
            const FileEntry *entry = f_context->getSourceManager().getFileEntryForID(i->first);
            string file = f_dg->getFile(entry->getName());
            std::error_code ec;
            sys::fs::remove(Twine(file));
            raw_fd_ostream rfo(StringRef(file), ec,
                               sys::fs::OpenFlags::F_Excl | sys::fs::OpenFlags::F_RW);
            //cout << "Trying to write " << entry->getName() << " to " << file << " with " << ec.message() << endl;
            i->second.write(rfo);
        }
    }

    bool AutoReplacer::VisitVarDecl(VarDecl *varDecl) {
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
    }
}

