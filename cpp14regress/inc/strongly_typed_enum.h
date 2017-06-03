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

    class ImprovedEnumReplacer : public FeatureVisitor {
    private:
        std::vector<clang::EnumDecl *> f_separatelyDefs;

        std::string nameForReplace() { return "enumeration"; }

    public:

        ImprovedEnumReplacer(clang::CompilerInstance *ci) : FeatureVisitor(ci) {}

        virtual features::type type() { return features::type::improved_enum; }

        virtual bool VisitEnumDecl(clang::EnumDecl *enumDecl);

        virtual bool VisitTypeLoc(clang::TypeLoc typeLoc);
    };

}

#endif /*CPP14REGRESS_STRONGLY_TYPED_ENUM_H*/