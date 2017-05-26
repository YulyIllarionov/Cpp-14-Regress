
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

    /*bool UniformInitReplacer::VisitVarDecl(clang::VarDecl *varDecl) {
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
    }*/

    //TODO delete
    /*bool UniformInitReplacer::VisitInitListExpr(InitListExpr *initListExpr) {
        if (!inProcessedFile(initListExpr, f_context))
            return true;
        //static string path = "/home/yury/llvm-clang/test/dot/";
        //static int i = 0;
        //ast_graph ag(initListExpr, f_context);
        //ag.to_dot_file(
        //        path + to_string(i++) + "  " + toString(initListExpr, f_context).substr(0, 25));
        return true;
    }*/

    //TODO best
    bool UniformInitReplacer::VisitCXXConstructExpr(CXXConstructExpr *constructExpr) {
        if (!fromUserFile(constructExpr, f_sourceManager))
            return true;
        if (constructExpr->isListInitialization() &&
            !constructExpr->isStdInitListInitialization()) {
            if (auto constructDecl = constructExpr->getConstructor()) {
                SourceRange braceRange = constructExpr->getParenOrBraceRange();
                if (constructExpr->getLocStart() == braceRange.getBegin()) {
                    f_rewriter->InsertTextBefore(braceRange.getBegin(),
                                                 constructDecl->getNameAsString());
                }
                SourceRange lParenRange = (braceRange.getBegin().getLocWithOffset(-1), braceRange.getBegin());
                SourceRange rParenRAnge = (braceRange.getEnd(), braceRange.getEnd().getLocWithOffset(1));
                f_rewriter->ReplaceText(lParenRange, "(");
                f_rewriter->ReplaceText(rParenRAnge, ")");
            }


            //static string path = "/home/yury/llvm-clang/test/dot/";
            //static int i = 0;
            //ast_graph ag(constructExpr, f_context);
            //ag.to_dot_file(
            //        path + to_string(i++) + " " + toString(constructExpr, f_context).substr(0, 25));
            //cout << "List constructor: " << toString(constructExpr, f_astContext) << " -- "
            //     << constructExpr->getConstructor()->getNameAsString();
            //if (auto constructDecl = constructExpr->getConstructor()) {
            //    cout << "<";
            //    if (auto tal = constructDecl->getTemplateSpecializationArgs()) {
            //        for (unsigned i = 0; i < tal->size(); i++) {
            //            QualType qt = (*tal)[i].getAsType();
            //            if (!qt.isNull()) {
            //                cout << qt.getAsString(PrintingPolicy(*f_langOptions));
            //            } else
            //                cout << "_";
            //            cout << ";";
            //        }
            //    }
            //    cout << ">";
            //}
            //cout << endl;
        }
        return true;
    }

    //TODO delete cause T() only
    /*bool UniformInitReplacer::VisitCXXScalarValueInitExpr(CXXScalarValueInitExpr *scalarInitExpr) {
        if (!inProcessedFile(scalarInitExpr, f_context))
            return true;

        cout << "Scalar init: " << toString(scalarInitExpr, f_context) << endl;
        return true;
    }*/
}
