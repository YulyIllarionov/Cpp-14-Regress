#ifndef CPP14REGRESS_STRONGLY_TYPED_ENUM_H
#define CPP14REGRESS_STRONGLY_TYPED_ENUM_H

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

#include "cpp14feature.h"

#include <iostream>
#include <vector>

namespace cpp14regress {

    class ImprovedEnumReplacer : public FeatureReplacer {
    private:
        std::vector<clang::EnumDecl *> f_enums;

        std::string nameForReplace() { return "enumeration"; }

    public:

        ImprovedEnumReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual features::type type() { return features::type::improved_enum; }

        virtual bool VisitEnumDecl(clang::EnumDecl *enumDecl);

        virtual bool VisitTypeLoc(clang::TypeLoc typeLoc);

        virtual void endSourceFileAction();
    };

    class EnumFieldSearcher : public clang::RecursiveASTVisitor<EnumFieldSearcher> {
    private:
        bool f_found = false;

    public:
        virtual bool VisitEnumConstantDecl(clang::EnumConstantDecl *field) {
            f_found = true;
            return false;
        }

        virtual bool VisitDecl(clang::Decl *decl) {
            if (decl)
                std::cout << decl->getDeclKindName() << std::endl;
            return true;
        }

        virtual bool VisitStmt(clang::Stmt *stmt) {
            if (stmt)
                std::cout << stmt->getStmtClassName() << std::endl;
            return true;
        }

        virtual bool VisitType(clang::Type *type) {
            if (type)
                std::cout << clang::QualType::getAsString(type, clang::Qualifiers()) << std::endl;
            return true;
        }

        bool found() { return f_found; }
    };
}

#endif /*CPP14REGRESS_STRONGLY_TYPED_ENUM_H*/