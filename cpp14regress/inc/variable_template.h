#ifndef CPP14REGRESS_INLINE_VARIABLE_TEMPLATE_H
#define CPP14REGRESS_INLINE_VARIABLE_TEMPLATE_H

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

    class VariableTemplateSearcher : public FeatureReplacer {
    public:

        VariableTemplateSearcher(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual features::type type() { return features::type::variable_templates; }

        virtual bool VisitVarTemplateDecl(clang::VarTemplateDecl *varTempl);
    };
}

#endif /*CPP14REGRESS_INLINE_VARIABLE_TEMPLATE_H*/