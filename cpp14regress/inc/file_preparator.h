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

    class FilesPreparator : public clang::ASTFrontendAction {
    public:
        class FilesPreparatorConsumer : public clang::ASTConsumer {
        public:
            FilesPreparatorConsumer() {}
        };

        explicit FilesPreparator(const std::string &folder, FilesList *files) :
                f_folder(asFolder(folder)), f_files(files) {}

        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                                      llvm::StringRef file) {
            FilesPreparatorConsumer *consumer = new FilesPreparatorConsumer();
            return std::unique_ptr<clang::ASTConsumer>(consumer);
        }

        virtual bool BeginSourceFileAction(clang::CompilerInstance &CI, llvm::StringRef Filename);

    private:
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