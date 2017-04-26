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

    RangeBasedForReplacer::RangeBasedForReplacer(ASTContext *context, cpp14features_stat *stat, DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void RangeBasedForReplacer::EndFileAction() {
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

    bool RangeBasedForReplacer::VisitCXXForRangeStmt(CXXForRangeStmt *for_loop) {

        ValueDecl *rangeVar = dyn_cast<DeclRefExpr>(for_loop->getRangeInit())->getDecl();
        ValueDecl *itVar = for_loop->getLoopVariable();
        string itStr = itVar->getQualifiedNameAsString();

        StringGenerator *sg;
        string forDecl;
        if (auto rangeVarType = dyn_cast<ConstantArrayType>(rangeVar->getType().getTypePtr())) {
            sg = new VariableToArrayElement(itVar, rangeVar);
            forDecl = "for (int " + itStr + " = 0; " + itStr + " < " + rangeVarType->getSize().toString(10, false) +
                      "; " + itStr + "++)"; //itn to size_t?
        } else {
            sg = new VariableToPointer(itVar);
            string rangeTypeStr = QualType::getAsString(rangeVar->getType().split());
            string rangeStr = rangeVar->getNameAsString();
            forDecl = "for (" + rangeTypeStr + "::iterator " + itStr + " = " + rangeStr + ".begin(); " +
                      itStr + " != " + rangeStr + ".end(); " + "++" + itStr + ")";
        }
        f_rewriter->ReplaceText(SourceRange(for_loop->getLocStart(), for_loop->getRParenLoc()), forDecl);

        RecursiveVariableReplacer *replacer = new RecursiveVariableReplacer(itVar, sg, f_rewriter);
        replacer->TraverseStmt(for_loop->getBody());
        return true;
    }

}