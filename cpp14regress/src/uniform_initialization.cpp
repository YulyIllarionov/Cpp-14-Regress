
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

    UniformInitReplacer::UniformInitReplacer(ASTContext *context,
                                             cpp14features_stat *stat, DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void UniformInitReplacer::EndFileAction() {
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
        //f_rewriter->overwriteChangedFiles();
    }

    bool UniformInitReplacer::VisitVarDecl(clang::VarDecl *varDecl) {
        if (!inProcessedFile(varDecl, f_context))
            return true;
        if (varDecl->hasInit()) {
            //static string path = "/home/yury/llvm-clang/test/dot/";
            //static int i = 0;
            //ast_graph ag(varDecl->getInit(), f_context);
            //ag.to_dot_file(path + string(varDecl->getInit()->getStmtClassName()) + to_string(i++));

            if (varDecl->getInitStyle() == VarDecl::InitializationStyle::ListInit) {
                StdListInitSearcher *searcher = new StdListInitSearcher();
                searcher->TraverseStmt(varDecl->getInit());
                if (!(searcher->found())) {
                    Expr *init = varDecl->getInit();
                    SourceRange braceRange;
                    if (auto ce = dyn_cast<CXXConstructExpr>(init)) {
                        braceRange = ce->getParenOrBraceRange();
                    } else if (auto lie = dyn_cast<InitListExpr>(init)) {
                        braceRange.setBegin(lie->getLBraceLoc());
                        braceRange.setEnd(lie->getRBraceLoc());
                    }
                    if (braceRange.isValid()) {
                        f_rewriter->ReplaceText(braceRange.getBegin(), 1, "(");
                        f_rewriter->ReplaceText(braceRange.getEnd(), 1, ")");
                    } else {
                        cout << "Uniform error: " << toString(varDecl, f_context) << " -- "
                             << varDecl->getInit()->getStmtClassName() << endl;
                    }
                }
            } else {
                StdListInitSearcher *stdSearcher = new StdListInitSearcher();
                stdSearcher->TraverseStmt(varDecl->getInit());
                ListInitSearcher *searcher = new ListInitSearcher(); //TODO unnecessary new
                searcher->TraverseStmt(varDecl->getInit());
                if ((searcher->found()) && !(stdSearcher->found())) {
                    cout << "Non list init: " << toString(varDecl, f_context) << " -- "
                         << varDecl->getLocation().printToString(f_context->getSourceManager())
                         << endl << console_hline('_') << endl;
                    //static string path = "/home/yury/llvm-clang/test/dot/";
                    //ast_graph ag(varDecl->getInit(), f_context);
                    //ag.to_dot_file(path + toString(varDecl, f_context).substr(0, 50));
                }
            }
        }
        return true;
    }

    //TODO delete
    bool UniformInitReplacer::VisitInitListExpr(InitListExpr *initListExpr) {
        if (!inProcessedFile(initListExpr, f_context))
            return true;
        //static string path = "/home/yury/llvm-clang/test/dot/";
        //static int i = 0;
        //ast_graph ag(initListExpr, f_context);
        //ag.to_dot_file(
        //        path + to_string(i++) + "  " + toString(initListExpr, f_context).substr(0, 25));
        return true;
    }

    //TODO best
    bool UniformInitReplacer::VisitCXXConstructExpr(CXXConstructExpr *constructExpr) {
        if (!inProcessedFile(constructExpr, f_context))
            return true;
        if (constructExpr->isListInitialization() &&
            !constructExpr->isStdInitListInitialization()) {

            //static string path = "/home/yury/llvm-clang/test/dot/";
            //static int i = 0;
            //ast_graph ag(constructExpr, f_context);
            //ag.to_dot_file(
            //        path + to_string(i++) + " " + toString(constructExpr, f_context).substr(0, 25));

            cout << "List constructor: " << toString(constructExpr, f_context) << " -- "
                 << constructExpr->getConstructor()->getNameAsString()
                 << endl; //TODO name if necessary
        }
        return true;
    }

    //TODO delete cause T() only
    bool UniformInitReplacer::VisitCXXScalarValueInitExpr(CXXScalarValueInitExpr *scalarInitExpr) {
        if (!inProcessedFile(scalarInitExpr, f_context))
            return true;

        cout << "Scalar init: " << toString(scalarInitExpr, f_context) << endl;
        return true;
    }
}
