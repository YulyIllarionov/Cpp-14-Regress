#ifndef CPP14REGRESS_INLINE_UNRESTRICTED_UNIONS_H
#define CPP14REGRESS_INLINE_UNRESTRICTED_UNIONS_H

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

#include "base_types.h"

namespace cpp14regress {

    class UnrestrictedUnionsSearcher : public FeatureReplacer {
    public:

        UnrestrictedUnionsSearcher(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual cpp14features type() { return cpp14features::unrestricted_unions; }

        virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl);
    };
}

#endif /*CPP14REGRESS_INLINE_UNRESTRICTED_UNIONS_H*/