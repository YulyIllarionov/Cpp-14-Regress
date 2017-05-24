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

#include "range_based_for.h"
#include "utils.h"

#include <string>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    string VariableToPointer::toString() {
        return string("(*" + f_variable->getNameAsString() + ")");
    }

    string VariableToArrayElement::toString() {
        return string(f_array->getNameAsString() + "[" + f_variable->getNameAsString() + "]");
    }

    bool RangeBasedForReplacer::VisitCXXForRangeStmt(CXXForRangeStmt *rbf) {
        if (!fromUserFile(rbf, f_sourceManager))
            return true;
        //TODO check null
        ValueDecl *rangeVar = dyn_cast<DeclRefExpr>(rbf->getRangeInit())->getDecl();
        ValueDecl *itVar = rbf->getLoopVariable();
        string itStr = itVar->getQualifiedNameAsString();

        StringGenerator *sg;
        string forDecl;
        if (auto rangeVarType = dyn_cast<ConstantArrayType>(rangeVar->getType().getTypePtr())) {
            sg = new VariableToArrayElement(itVar, rangeVar);
            forDecl = "for (unsigned " + itStr + " = 0; "
                      + itStr + " < " + rangeVarType->getSize().toString(10, false) + "; "
                      + itStr + "++)"; //int to size_t?
        } else {
            sg = new VariableToPointer(itVar);
            string rangeTypeStr = rangeVar->getType().getAsString(PrintingPolicy(*f_langOptions));
            string rangeVarStr = rangeVar->getNameAsString();
            //TODO change auto iterator type
            forDecl = "for (auto " + itStr + " = " + rangeVarStr
                      + ".begin(); " + itStr + " != " + rangeVarStr
                      + ".end(); " + "++" + itStr + ")";
        }
        f_rewriter->ReplaceText(SourceRange(rbf->getLocStart(), rbf->getRParenLoc()), forDecl);
        VariableReplacer vr(itVar, sg, f_rewriter);
        vr.TraverseStmt(rbf->getBody());
        f_rewriter->InsertTextBefore(rbf->getLocStart(), Comment::line(
                replacement::info(type(), replacement::result::replaced)) + "\n");
        return true;
    }

}