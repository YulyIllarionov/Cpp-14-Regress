#ifndef CPP14REGRESS_CONSTRUCTOR_INHERITING_H
#define CPP14REGRESS_CONSTRUCTOR_INHERITING_H

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

namespace cpp14regress {

    struct ArgName {
        static std::string toString(unsigned i) {
            return std::string("arg" + std::to_string(i));
        }
    };

    class ConstructorInheritingReplacer : public FeatureVisitor {
    private:
        typedef NameGenerator<ArgName> ArgNameGenerator;

    public:

        ConstructorInheritingReplacer(clang::CompilerInstance *ci) : FeatureVisitor(ci) {}

        virtual features::type type() { return features::type::constructor_inheriting; }

        virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl);

    };
}

#endif /*CPP14REGRESS_VARIADIC_TEMPLATE_H*/