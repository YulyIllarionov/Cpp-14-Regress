#ifndef CPP14REGRESS_FUNC_TEMPL_DEFAULT_ARGS_H
#define CPP14REGRESS_FUNC_TEMPL_DEFAULT_ARGS_H

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

    class FuncTemplDefaultArgsSearcher : public FeatureVisitor {
    public:

        FuncTemplDefaultArgsSearcher(clang::CompilerInstance *ci) : FeatureVisitor(ci) {}

        virtual features::type type() { return features::type::trailing_return; }

        virtual bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl *functionTempl);

    };
}

#endif /*CPP14REGRESS_FUNC_TEMPL_DEFAULT_ARGS_H*/