
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

    AliasTypeReplacer::AliasTypeReplacer(ASTContext *context,
                                         cpp14features_stat *stat, DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void AliasTypeReplacer::EndFileAction() {
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

    bool AliasTypeReplacer::VisitTypeAliasDecl(clang::TypeAliasDecl *aliasTypeDecl) {
        if (!inProcessedFile(aliasTypeDecl, f_context))
            return true;

        if (!(aliasTypeDecl->getDescribedAliasTemplate())) {
            const SourceManager &sm = f_context->getSourceManager();
            string oldTypedef = "typedef ";
            QualType qt = aliasTypeDecl->getUnderlyingType();
            string synonym = aliasTypeDecl->getNameAsString();
            if (qt->isFunctionPointerType()) {
                string type = qt.getAsString();
                string::size_type pos = type.find("(*)"); //TODO fix
                if (pos == string::npos) {
                    //TODO comment can't understand
                    cerr << "alias type error" << endl;
                    return true;
                }
                type.insert(pos + 2, synonym);
                oldTypedef += type;
            } else {
                oldTypedef += toString(aliasTypeDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange(),
                                       f_context);
                oldTypedef += " ";
                oldTypedef += synonym;
            }
            oldTypedef += ";";
            SourceRange sr = aliasTypeDecl->getSourceRange();
            sr.setEnd(Lexer::getLocForEndOfToken(sr.getEnd(), 0,
                                                 sm, f_context->getLangOpts()));
            f_rewriter->ReplaceText(sr, oldTypedef);
        }
        //else {
        //    cout << "Alias template: " << toString(aliasTemplate, f_context) /*<< " -- "
        //         << aliasTemplateDecl->getLocation().printToString(f_context->getSourceManager())*/ << endl;
        //    cout << console_hline('_') << endl;
        //}
        return true;
    }
}