
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

    void FilesPreparatorConsumer::EndSourceFileAction() {
        const SourceManager &sm = f_ci.getSourceManager();
        string mainPath(sm.getFileEntryForID(sm.getMainFileID())->getName());
        f_files->push_back(mainPath);
        string folder = f_folder + asFolder(removeExtension(pathPopBack(mainPath)));
        cout << folder << endl << sys::fs::create_directory(folder).message() << endl;
        //for (auto it = sm.fileinfo_begin(); it != sm.fileinfo_end(); it++) {
        //    if (sm.getFileCharacteristic(sm.getLocForStartOfFile(sm.translateFile(it->first))) ==
        //        clang::SrcMgr::CharacteristicKind::C_User) {
        //        string file = it->first->getName();
        //        //error_code ec = sys::fs::copy_file(it->first->getName(),
        //        //                                   folder + pathPopBack(file));
        //        //cout << ec.message() << endl;
        //    }
        //}
    }
}