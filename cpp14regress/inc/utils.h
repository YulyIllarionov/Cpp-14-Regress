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

    class RecursiveVariableReplacer : public clang::RecursiveASTVisitor<RecursiveVariableReplacer> {
    private:
        const clang::ValueDecl *f_variable;
        StringGenerator *f_generator;
        clang::Rewriter *f_rewriter;

    public:
        RecursiveVariableReplacer(clang::ValueDecl *variable_,
                                  StringGenerator *generator_, clang::Rewriter *rewriter_) :
                f_variable(variable_), f_generator(generator_), f_rewriter(rewriter_) {}

        virtual bool VisitDeclRefExpr(clang::DeclRefExpr *dre);
    };


    std::string toString(clang::SourceRange sr, const clang::ASTContext &context);

    template<typename T>
    std::string toString(T *source, const clang::ASTContext &context) {
        clang::SourceRange sr(source->getLocStart(), source->getLocEnd());
        return toString(sr, context);
    }

    //std::string toString(clang::SourceRange sr, const clang::ASTContext *context) {
    //    std::cerr << "This toString() is deprecated" << std::endl;
    //    return std::string();
    //}

    template<typename T>
    std::string toString(T *source, const clang::ASTContext *context) {
        std::cerr << "This toString() is deprecated" << std::endl;
        return std::string();
    }

    template<typename T>
    inline bool fromSystemFile(T *source, const clang::ASTContext &context) {
        clang::SourceLocation sl = source->getLocStart();
        if (sl.isValid())
            if (context.getSourceManager().getFileCharacteristic(sl) !=
                clang::SrcMgr::CharacteristicKind::C_User)
                return true;
        return false;
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

    class Indent {
    private:
        unsigned int f_level;
        const static unsigned int f_size = 4;
    public:
        Indent() : f_level(0) {}

        Indent(const Indent &other) = default;

        Indent &operator++();

        Indent operator++(int);

        Indent &operator--();

        Indent operator--(int);

        operator std::string() const { return std::string(f_level * f_size, ' '); }

        friend std::ostream &operator<<(std::ostream &stream, const Indent &indent);
    };

    std::vector<std::string> filesInFolder(std::string folder);

    bool isCppFile(std::string filename);

    bool isCppSourceFile(std::string filename);

    clang::SourceRange
    getParamRange(const clang::FunctionDecl *func, const clang::ASTContext *context); //TODO improve

    std::vector<std::string> pathToVector(std::string path);

    std::string pathPopBack(std::string &path);

    std::string removeExtension(const std::string &path);

    std::string asFolder(const std::string &path);

    inline std::string VariableToField(const std::string &var) {
        return std::string("f_" + var);
    }

    class NameGenerator {
    protected:
        unsigned f_count = 0;
        bool f_first = true;
    public:

        virtual std::string toString() { return std::string(); };

        std::string generate();

        void reset();
    };

}

#endif /*CPP14REGRESS_UTILS_H*/
