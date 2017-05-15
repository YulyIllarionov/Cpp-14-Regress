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

    bool RangeBasedForReplacer::VisitCXXForRangeStmt(CXXForRangeStmt *for_loop) {

        ValueDecl *rangeVar = dyn_cast<DeclRefExpr>(for_loop->getRangeInit())->getDecl();
        ValueDecl *itVar = for_loop->getLoopVariable();
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
            string rangeTypeStr = QualType::getAsString(rangeVar->getType().split());
            string rangeStr = rangeVar->getNameAsString();
            forDecl = "for (" + rangeTypeStr + "::iterator " + itStr + " = " + rangeStr
                      + ".begin(); " + itStr + " != " + rangeStr
                      + ".end(); " + "++" + itStr + ")";
        }
        rewriter()->ReplaceText(SourceRange(for_loop->getLocStart(), for_loop->getRParenLoc()),
                                forDecl);

        RecursiveVariableReplacer *replacer = new RecursiveVariableReplacer(itVar, sg, rewriter());
        replacer->TraverseStmt(for_loop->getBody());
        return true;
    }

}