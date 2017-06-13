#ifndef CPP14REGRESS_FILE_PREPARATOR_H
#define CPP14REGRESS_FILE_PREPARATOR_H

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

#include "utils.h"

#include <iostream>
#include <string>
#include <vector>

namespace cpp14regress {

    typedef std::vector<std::string> FilesList;

    class FilesPreparatorConsumer : public clang::ASTConsumer {
    public:
        FilesPreparatorConsumer(clang::CompilerInstance &ci, const std::string &folder,
                                FilesList *files) : f_ci(ci), f_folder(folder), f_files(files) {}

        void EndSourceFileAction();

    private:
        clang::CompilerInstance &f_ci;
        std::string f_folder;
        FilesList *f_files;
    };

    class FilesPreparator : public clang::ASTFrontendAction {
    public:

        FilesPreparator(const std::string &folder, FilesList *files) :
                f_folder(asFolder(folder)), f_files(files) {}

        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &ci,
                                                                      llvm::StringRef file) {
            f_consumer = new FilesPreparatorConsumer(ci, f_folder, f_files);
            return std::unique_ptr<clang::ASTConsumer>(f_consumer);
        }

        virtual void EndSourceFileAction() { f_consumer->EndSourceFileAction(); }

    private:
        FilesPreparatorConsumer *f_consumer;
        std::string f_folder;
        FilesList *f_files;
    };

    class FilesPreparatorFactory : public clang::tooling::FrontendActionFactory {
    public:
        explicit FilesPreparatorFactory(const std::string &folder, FilesList *files) :
                f_folder(folder), f_files(files) {}

        clang::FrontendAction *create() {
            return new FilesPreparator(f_folder, f_files);
        }

    private:
        std::string f_folder;
        FilesList *f_files;
    };
}
#endif /*CPP14REGRESS_FILE_PREPARATOR_H*/