
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

#include "in_class_init.h"
#include "utils.h"
#include "ast_to_dot.h"

#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    MemberInitReplacer::MemberInitReplacer(ASTContext *context, cpp14features_stat *stat, DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void MemberInitReplacer::EndFileAction() {
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

    bool MemberInitReplacer::VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl) {
        if (!inProcessedFile(recordDecl, f_context))
            return true;

        const SourceManager &sm = f_context->getSourceManager();
        const LangOptions &lo = f_context->getLangOpts();

        bool initFound = false;
        for (auto field = recordDecl->field_begin(); field != recordDecl->field_end(); field++) {
            if (field->hasInClassInitializer()) {
                SourceLocation b = Lexer::getLocForEndOfToken(field->getLocation(), 0, sm, lo);
                SourceLocation e = Lexer::getLocForEndOfToken(field->getLocEnd(), 0, sm, lo);
                f_rewriter->ReplaceText(SourceRange(b, e), ";");
                initFound = true;
            }
        }
        if (!initFound)
            return true;

        if (recordDecl->ctor_begin() == recordDecl->ctor_end()) {
            //TODO add constructor
            return true;
        }

        for (auto declaration = recordDecl->ctor_begin();
             declaration != recordDecl->ctor_end(); declaration++) {

            const FunctionDecl *fd = nullptr;
            declaration->isDefined(fd);
            const CXXConstructorDecl *definition = dyn_cast_or_null<CXXConstructorDecl>(fd);
            if (definition) {

                auto last = find_if(definition->init_rbegin(), definition->init_rend(),
                                    [](CXXCtorInitializer *init) {
                                        return (*init).isWritten();
                                    });
                SourceLocation initBegin = Lexer::getLocForEndOfToken(
                        getParamRange(fd, f_context).getEnd(), 0, sm, lo);
                SourceLocation initEnd = (last == definition->init_rend()) ? initBegin :
                                         definition->getBody()->getLocStart().getLocWithOffset(-1); //TODO fix

                bool replace = false;
                vector<string> inits;
                for (auto init = definition->init_begin(); init != definition->init_end(); init++) {
                    if ((*init)->isWritten()) {
                        inits.push_back(toString((*init)->getSourceRange(), f_context));
                    } else if ((*init)->isInClassMemberInitializer()) {
                        if (auto field = (*init)->getMember()) {
                            string init(field->getNameAsString() + "(" + //TODO fix: only value
                                        toString(field->getInClassInitializer(), f_context) + ")");
                            inits.push_back(init);
                            replace = true;
                        } else {/*TODO unrecognized*/}
                    }
                }

                if (replace) {
                    string initSource = " : ";
                    for (auto init = inits.begin();;) {
                        initSource += *init;
                        if (++init != inits.end())
                            initSource += ", ";
                        else break;
                    }
                    f_rewriter->ReplaceText(SourceRange(initBegin, initEnd), initSource);
                }
            }
        }

        return true;
    }
}
