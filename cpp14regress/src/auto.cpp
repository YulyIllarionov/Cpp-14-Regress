
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
#include "utils.h"
#include "ast_to_dot.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    AutoReplacer::AutoReplacer(ASTContext *context) {
        f_context = context;
        f_rewriter = new Rewriter(context->getSourceManager(),
                                  context->getLangOpts());
    }

    bool AutoReplacer::VisitDeclaratorDecl(DeclaratorDecl *valueDecl) {
        if (f_context->getSourceManager().isInSystemHeader(valueDecl->getLocStart()))
            return true;
        auto at = dyn_cast<AutoType>(valueDecl->getType().getTypePtr());
        if (!at)
            return true;

        static bool first = true;
        if (first) {
            cout << "------------" << endl;
            first = false;
        }

        cout << toSting<>(valueDecl, f_context) << endl;
        auto tl = valueDecl->getTypeSourceInfo()->getTypeLoc();
        cout << toSting(&tl, f_context) << endl;
        QualType qt = at->getDeducedType(); //TODO understand deduced
        if (!qt.isNull()) {
            if (valueDecl->isFunctionOrFunctionTemplate()) {
                cout << "function decl" << endl;
            } else {
                cout << qt.getAsString() << " "
                     << valueDecl->getNameAsString() << endl;
            }
            const Type *t = qt.getTypePtr();
            if (isa<DependentNameType>(t)
                || isa<DependentTemplateSpecializationType>(t)
                || isa<ElaboratedType>(t)) {
                cout << "Keyword Type" << endl;
            }
        } else
            cout << "Not deduced" << endl;
        cout << "------------" << endl;


        //if (auto dnt = dyn_cast<DependentNameType>(qt.getTypePtr())) {
        //    cout << "DependentNameType" << endl;
        //}
        //if (auto dtst = dyn_cast<DependentTemplateSpecializationType>(qt.getTypePtr())) {
        //    cout << "DependentTemplateSpecializationType" << endl;
        //}
        //if(auto et = dyn_cast<ElaboratedType>(qt.getTypePtr()))
        //cout << "---" << et->getKeywordName(et->getKeyword()).str() << endl;
        return true;
    }
}

