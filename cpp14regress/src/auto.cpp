
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

    AutoReplacer::AutoReplacer(ASTContext *context, cpp14features_stat *stat)
            : f_context(context), f_stat(stat) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    bool AutoReplacer::VisitDeclaratorDecl(DeclaratorDecl *declaratorDecl) {
        if (!inProcessedFile(declaratorDecl, f_context))
            return true;
        if (auto at = dyn_cast<AutoType>(declaratorDecl->getType().getTypePtr())) {
            //TODO need GNUAutoType?
            QualType qt = at->getDeducedType(); //TODO understand deduced
            if (!qt.isNull()) {
                string typeName = qt.getAsString();
                SourceRange typeRange = declaratorDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange();
                if (qt->isFunctionPointerType()) { //TODO auto a, b, c; ???
                    size_t pointerPos = typeName.find("(*)"); //TODO change
                    if (pointerPos != string::npos) {
                        typeName.insert(pointerPos + 2, declaratorDecl->getNameAsString());
                    } else {
                        cerr << "auto tool error" << endl;
                        return false;
                    }
                    typeRange.setEnd(Lexer::getLocForEndOfToken(declaratorDecl->getLocation(), 0,
                                                                f_context->getSourceManager(),
                                                                f_context->getLangOpts()));

                    //typeRange.setEnd(declaratorDecl->loc)
                    //cout << qt.getAsString() << " -- " << qt->getTypeClassName();
                    //if (auto fpt = dyn_cast_or_null<FunctionProtoType>(qt->getPointeeType().getTypePtr())) {
                    //    cout << " -- " << fpt->getLocallyUnqualifiedSingleStepDesugaredType().getAsString()
                    //         << " -- " << fpt->getTypeClassName() << endl;
                    //    for (auto it = fpt->param_type_begin(); it != fpt->param_type_end(); it++) {
                    //        cout << it->getAsString() << " -- " << (*it)->getTypeClassName() << endl;
                    //    }
                    //}
                    //cout << endl;
                }
                f_rewriter->ReplaceText(typeRange, typeName);
            }
            else {
                //TODO not deduced
            }
        }
        return true;
    }
}

