
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

#include "user_literals.h"
#include "utils.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    UserLiteralReplacer::UserLiteralReplacer(ASTContext *context,
                                             cpp14features_stat *stat, DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void UserLiteralReplacer::EndFileAction() {
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

    bool UserLiteralReplacer::VisitFunctionDecl(clang::FunctionDecl *funcDecl) {
        if (!inProcessedFile(funcDecl, f_context))
            return true;

        if (funcDecl->getNameAsString().find("\"\"") != string::npos) { //TODO fix
            cout << "User-defined literal" << endl << toString(funcDecl, f_context) << endl
                 << funcDecl->getType()->getTypeClassName() << endl;
            if (auto ii = funcDecl->getLiteralIdentifier()) {
                cout << string(ii->getName()) << endl;
            }
        }
        return true;
    }

    bool UserLiteralReplacer::VisitUserDefinedLiteral(clang::UserDefinedLiteral *literal) {
        if (!inProcessedFile(literal, f_context))
            return true;
        cout << "User literal: " << toString(literal->getCallee(), f_context) << endl;
        return true;
    }

}