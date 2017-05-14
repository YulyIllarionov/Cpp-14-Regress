
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

#include "file_preparator.h"
#include "utils.h"
#include <iostream>
#include <string>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool FilesPreparator::BeginSourceFileAction(CompilerInstance &CI, StringRef Filename) {
        const ASTContext &context = CI.getASTContext();
        SourceManager &sm = CI.getASTContext().getSourceManager();
        const LangOptions &lo = CI.getASTContext().getLangOpts();
        Rewriter rewriter(sm, lo);
        std::set<FileID> fileIds;
        const FileEntry *mainEntry;
        for (auto it = sm.fileinfo_begin(); it != sm.fileinfo_end(); it++) {
            FileID id = sm.translateFile(it->first);
            if (id == sm.getMainFileID())
                mainEntry = it->first;
            else
                fileIds.insert(id);
        }
        string filename = pathPopBack(string(mainEntry->getName()));
        string folder = f_folder + removeExtension(filename) + '/';

        for (auto fileId : fileIds) {
            if (fileId.isValid()) {
                if (sm.getFileCharacteristic(sm.getLocForStartOfFile(fileId)) ==
                    clang::SrcMgr::CharacteristicKind::C_User) {
                    SourceLocation includeBeginLoc = sm.getIncludeLoc(fileId);
                    if (includeBeginLoc.isValid()) {
                        SourceLocation includeEndLoc = Lexer::getLocForEndOfToken(
                                includeBeginLoc, 0, sm, context.getLangOpts());
                        string inc("\"" + pathPopBack(sm.getFileEntryForID(fileId)->getName()) +
                                   "\"");
                        rewriter.ReplaceText(SourceRange(includeBeginLoc, includeEndLoc), inc);
                    }
                    for (auto it = rewriter.buffer_begin(); it != rewriter.buffer_end(); it++) {
                        sys::fs::copy_file(sm.getFileEntryForID(it->first), folder);
                    }
                }
            }
        }
    }
}