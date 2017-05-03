
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

#include "strongly_typed_enum.h"
#include "utils.h"

#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    StronglyTypedEnumReplacer::StronglyTypedEnumReplacer(ASTContext *context,
                                                         cpp14features_stat *stat,
                                                         DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void StronglyTypedEnumReplacer::EndFileAction() {
        for (auto enumDecl : f_enumDecls) {
            cout << enumDecl << endl << toString(enumDecl, f_context) << endl;
        }
        cout << console_hline('-') << endl;

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

    bool StronglyTypedEnumReplacer::VisitValueDecl(ValueDecl *varDecl) {
        if (!inProcessedFile(varDecl, f_context))
            return true;
        if (auto enumType = dyn_cast_or_null<EnumType>(varDecl->getType().getTypePtr())) {
            if (EnumDecl *enumDecl = enumType->getDecl()) {
                cout << "Enum variable: " << toString(varDecl, f_context) << endl;
                f_enumDecls.insert(enumDecl);
            }
        }
        return true;
    }

    bool StronglyTypedEnumReplacer::VisitEnumDecl(EnumDecl *enumDecl) {
        if (!inProcessedFile(enumDecl, f_context))
            return true;

        if (EnumDecl *enumDef = enumDecl->getDefinition()) {
            const SourceManager &sm = f_context->getSourceManager();
            const LangOptions &lo = f_context->getLangOpts();
            //Remove enumeration type
            SourceLocation typeEnd = enumDef->getIntegerTypeRange().getEnd();
            if (typeEnd.isValid()) {
                SourceLocation nameEnd = Lexer::getLocForEndOfToken(enumDef->getLocation(),
                                                                    0, sm, lo);
                //f_rewriter->RemoveText(SourceRange(nameEnd, typeEnd));
            }
            //Remove last comma
            //if (enumDef->enumerator_begin() != enumDef->enumerator_end()) {
            //    auto last = enumDef->enumerator_begin();
            //    while (std::next(last) != enumDef->enumerator_end()) last++;
            //    SourceLocation afterLast = last->getLocEnd(), commaBegin;
            //    Token token;
            //    do {
            //        afterLast = Lexer::AdvanceToTokenCharacter(afterLast, 0, sm, lo);
            //        cout << "End: " << afterLast.printToString(sm) << endl;
            //        if (Lexer::getRawToken(afterLast, token, sm, lo)) {
            //            //TODO not understand
            //            cerr << "Strongly typed enum replacer error" << endl;
            //            break;
            //        }
            //        if (token.is(tok::TokenKind::comma)) {
            //            commaBegin = afterLast;
            //            break;
            //        }
            //    } while (token.isNot(tok::TokenKind::r_brace));
            //    if (commaBegin.isValid()) {
            //        cout << "Last comma:" << commaBegin.printToString(sm) << endl;
            //    }
            //}
        }

        return true;
    }
}