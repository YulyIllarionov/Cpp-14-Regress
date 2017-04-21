
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

#include "digit_separators.h"
#include "utils.h"

#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    DigitSeparatorReplacer::DigitSeparatorReplacer(ASTContext *context, cpp14features_stat *stat,
                                                   DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void DigitSeparatorReplacer::EndFileAction() {
        for (auto i = f_rewriter->buffer_begin(), e = f_rewriter->buffer_end(); i != e; ++i) {
            const FileEntry *entry = f_context->getSourceManager().getFileEntryForID(i->first);
            string file = f_dg->getFile(entry->getName());
            std::error_code ec;
            sys::fs::remove(Twine(file));
            raw_fd_ostream rfo(StringRef(file), ec, sys::fs::OpenFlags::F_Excl | sys::fs::OpenFlags::F_RW);
            //cout << "Trying to write " << entry->getName() << " to " << file << " with " << ec.message() << endl;
            i->second.write(rfo);
        }
    }

    bool DigitSeparatorReplacer::VisitIntegerLiteral(IntegerLiteral *literal) {
        if (!inProcessedFile(literal, f_context))
            return true;
        string i = toString(literal, f_context);
        if (removeSeparators(i)) {
            //cout << toString(literal, f_context) << endl;
            f_rewriter->ReplaceText(literal->getSourceRange(), i);
        }
        return true;
    }

    bool DigitSeparatorReplacer::VisitFloatingLiteral(clang::FloatingLiteral *literal) {
        if (!inProcessedFile(literal, f_context))
            return true;
        string f = toString(literal, f_context);
        if (removeSeparators(f)) {
            //cout << toString(literal, f_context) << endl;
            f_rewriter->ReplaceText(literal->getSourceRange(), f);
        }
        return true;
    }

    bool DigitSeparatorReplacer::removeSeparators(std::string &literal) {
        bool found = false;
        string::size_type pos = literal.find('\'');
        while (pos != string::npos) {
            literal.erase(pos, 1);
            pos = literal.find('\'', pos);
            if (!found) found = true;
        }
        return found;
    }

}
