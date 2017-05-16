
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

#include "constuctor_delegation.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    string ConstructorDelegationReplacer::initFunName(const CXXConstructorDecl *ctr) {
        return (ctr->getNameAsString() + f_seed);
    }

    string ConstructorDelegationReplacer::initFunCall(const CXXConstructorDecl *delegating) {
        string call;
        const SourceManager &sm = f_context->getSourceManager();
        SourceLocation b((*(delegating->init_begin()))->getLParenLoc());
        SourceLocation e((*(delegating->init_begin()))->getRParenLoc().getLocWithOffset(1));
        string params(sm.getCharacterData(b), sm.getCharacterData(e));
        return string(initFunName(delegating) + params + ";");
    }

    ConstructorDelegationReplacer::ConstructorDelegationReplacer(ASTContext *context,
                                                                 cpp14features_stat *stat, DirectoryGenerator *dg)
            : f_context(context), f_stat(stat), f_dg(dg) {
        f_rewriter = new Rewriter(context->getSourceManager(), //TODO delete in destructor
                                  context->getLangOpts());
    }

    void ConstructorDelegationReplacer::EndFileAction() {
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

    bool ConstructorDelegationReplacer::VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl) {
        if (!inProcessedFile(recordDecl, f_context))
            return true;
        if (recordDecl->ctor_begin() == recordDecl->ctor_end())
            return true;
        vector<CXXConstructorDecl *> targetCtors;
        const SourceManager &sm = f_context->getSourceManager();
        const LangOptions &lo = f_context->getLangOpts();
        for (auto declaration = recordDecl->ctor_begin();
             declaration != recordDecl->ctor_end(); declaration++) {

            const FunctionDecl *fd = nullptr;
            declaration->isDefined(fd);
            const CXXConstructorDecl *definition = dyn_cast_or_null<CXXConstructorDecl>(fd);

            if (definition) {
                //if (definition->isUserProvided()) {
                //    cout << console_hline('-') << endl
                //         << toString(*declaration, f_context) << " -- "
                //         << toString(definition, f_context) << endl
                //         << console_hline('-') << endl;
                //    cout << *declaration << " -- " << definition << endl;
                //}

                if (definition->isDelegatingConstructor()) {

                    auto targetCtor = definition->getTargetConstructor();
                    auto pos = find(targetCtors.begin(), targetCtors.end(), targetCtor); //TODO fix duplicates check
                    if (pos == targetCtors.end()) {
                        targetCtors.push_back(targetCtor);
                    }
                    //TODO check multiple init
                    SourceLocation bodyStart = Lexer::getLocForEndOfToken(
                            definition->getBody()->getLocStart(), 0, sm, lo);
                    string call("\n" + initFunCall(definition));
                    f_rewriter->InsertText(bodyStart, call, true, true);
                    SourceRange paramRange = getParamRange(dyn_cast<FunctionDecl>(definition), *f_context);
                    f_rewriter->RemoveText(SourceRange(paramRange.getEnd().getLocWithOffset(1),
                                                       definition->getBody()->getLocStart().getLocWithOffset(-1)));
                }
            }
        }
        CXXRecordDecl::method_iterator last;
        for (auto it = recordDecl->method_begin(); it != recordDecl->method_end(); it++)
            if (it->isUserProvided())
                last = it;
        SourceLocation initFunInsert = recordDecl->getRBraceLoc();
        if (!targetCtors.empty()) {
            f_rewriter->InsertText(initFunInsert, "\nprivate:");
        }
        for (auto declaration = targetCtors.rbegin();
             declaration != targetCtors.rend(); declaration++) {
            const FunctionDecl *fd;
            (*declaration)->isDefined(fd);
            const CXXConstructorDecl *definition = dyn_cast_or_null<CXXConstructorDecl>(fd);

            if (definition) {
                SourceRange paramRange = getParamRange(dyn_cast<FunctionDecl>((*declaration)), *f_context);
                string initFunParams(sm.getCharacterData(paramRange.getBegin()),
                                     sm.getCharacterData(paramRange.getEnd()) + 1);
                string initFunBody = "{";
                int size = 0;
                for (auto it = definition->init_begin(); it != definition->init_end(); it++) {
                    size++;
                    string init("\n");
                    if ((*it)->isDelegatingInitializer()) {
                        init += initFunCall(definition);
                    } else {
                        init += (*it)->getMember()->getNameAsString();
                        init += " = ";
                        const char *b = sm.getCharacterData((*it)->getLParenLoc()) + 1;
                        const char *e = sm.getCharacterData((*it)->getRParenLoc());
                        init += string(b, e - b);
                        init += ";";
                    }
                    initFunBody += init;
                }
                SourceLocation bodyStart = Lexer::getLocForEndOfToken(
                        definition->getBody()->getLocStart(), 0, sm, lo);
                initFunBody += string(sm.getCharacterData(bodyStart),
                                      sm.getCharacterData(definition->getBody()->getLocEnd()) -
                                      sm.getCharacterData(bodyStart));
                initFunBody += "}";
                string initFunDefinition(" \n" + initFunName(definition) +
                                         initFunParams + " " + initFunBody + "\n");
                f_rewriter->InsertText(initFunInsert, initFunDefinition, true, true);
            }
        }
        return true;
    }
}
