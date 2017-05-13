
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
#include <set>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    void FilesPreparator::EndFileAction() {
        std::set<FileID> fileIds;
        for (auto it = f_sm.fileinfo_begin(); it != f_sm.fileinfo_end(); it++) {
            fileIds.insert(f_sm.translateFile(it->first));
        }

        for (auto fileId : fileIds) {
            if (fileId.isValid()) {
                if (f_sm.getFileCharacteristic(f_sm.getLocForStartOfFile(fileId)) ==
                    clang::SrcMgr::CharacteristicKind::C_User) {
                    if (fileId == f_sm.getMainFileID())
                        cout << " \t";
                    cout << f_sm.getFileEntryForID(fileId)->getName();
                    SourceLocation includeBeginLoc = f_sm.getIncludeLoc(fileId);
                    if (includeBeginLoc.isValid()) {
                        SourceLocation includeEndLoc = Lexer::getLocForEndOfToken(
                                includeBeginLoc, 0, f_sm, f_context->getLangOpts());
                        cout << " -- " << toString(SourceRange(includeBeginLoc, includeEndLoc),
                                                   f_context);
                    }
                    cout << endl;
                }
            }
        }
        cout << console_hline() << endl;
    }

}