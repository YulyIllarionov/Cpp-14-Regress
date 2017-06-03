#ifndef CPP14REGRESS_UTILS_H
#define CPP14REGRESS_UTILS_H

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

#include <string>
#include <iostream>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>

namespace cpp14regress {

    std::string console_hline(char c = '*');

    class StringGenerator {
    public:
        virtual std::string toString() = 0;
    };

    template<class SG>
    class NameGenerator {
    public:
        static unsigned f_count;
    public:
        static std::string toString() { return SG::toString(f_count); }

        static void generate() { f_count++; }

        static void reset() { f_count = 0; }
    };

    template<class SG>
    unsigned NameGenerator<SG>::f_count = 0;

    class VariableReplacer : public clang::RecursiveASTVisitor<VariableReplacer> {
    private:
        const clang::ValueDecl *f_variable;
        StringGenerator *f_generator;
        clang::Rewriter *f_rewriter;

    public:
        VariableReplacer(clang::ValueDecl *vd, StringGenerator *sg, clang::Rewriter *r) :
                f_variable(vd), f_generator(sg), f_rewriter(r) {}

        virtual bool VisitDeclRefExpr(clang::DeclRefExpr *dre);
    };


    std::string toString(clang::SourceRange sr, const clang::ASTContext *context,
                         bool tokenEnd = true);

    template<typename T>
    std::string toString(T *source, const clang::ASTContext *context) {
        clang::SourceRange sr(source->getLocStart(), source->getLocEnd());
        return toString(sr, context);
    }

    template<typename T>
    inline bool fromUserFile(T *source, const clang::SourceManager *sm) {
        clang::SourceLocation sl = source->getLocStart();
        if (sl.isValid())
            if (sm->getFileCharacteristic(sl) !=
                clang::SrcMgr::CharacteristicKind::C_User)
                return false;
        return true;
    }

    template<>
    inline bool fromUserFile<clang::Attr>(clang::Attr *source, const clang::SourceManager *sm) {
        clang::SourceLocation sl = source->getLocation();
        if (sl.isValid())
            if (sm->getFileCharacteristic(sl) !=
                clang::SrcMgr::CharacteristicKind::C_User)
                return false;
        return true;
    }

    template<typename T>
    inline bool inProcessedFile(T *source, clang::ASTContext *context) {
        if (source->getLocStart().isValid())
            if (context->getSourceManager().getFileCharacteristic(source->getLocStart()) ==
                clang::SrcMgr::CharacteristicKind::C_User)
                return true;
        return false;
    }

    bool isCppFile(const clang::Twine &path);

    std::vector<std::string> filesInFolder(std::string folder);

    bool isCppFile(std::string filename);

    bool isCppSourceFile(std::string filename);

    clang::SourceRange
    getParamRange(const clang::FunctionDecl *func, const clang::ASTContext *context); //TODO improve

    std::vector<std::string> pathToVector(std::string path);

    std::string pathPopBack(std::string &path);

    std::string removeExtension(const std::string &path);

    std::string asFolder(const std::string &path);

    clang::SourceLocation getIncludeLocation(clang::FileID fileID, const clang::SourceManager *sm,
                                             unsigned carriages = 5);

    std::vector<std::string> getIncludes(clang::FileID fileID, const clang::ASTContext &context);

    clang::SourceLocation findTokenEndAfterLoc(clang::SourceLocation start, clang::tok::TokenKind kind,
                                               const clang::ASTContext *context,
                                               bool skipWhitespace = false);

    clang::SourceLocation findTokenBeginAfterLoc(clang::SourceLocation start, clang::tok::TokenKind kind,
                                                 unsigned size, const clang::ASTContext *context);

    clang::SourceLocation findTokenEndBeforeLoc(clang::SourceLocation start, clang::tok::TokenKind kind,
                                                const clang::ASTContext *context,
                                                bool skipWhitespace = false);

    clang::SourceLocation findTokenBeginBeforeLoc(clang::SourceLocation start, clang::tok::TokenKind kind,
                                                  unsigned size, const clang::ASTContext *context);

    clang::SourceLocation findTokenLoc(clang::SourceRange sr, const clang::ASTContext &context,
                                       clang::tok::TokenKind kind, unsigned size);
}

#endif /*CPP14REGRESS_UTILS_H*/
