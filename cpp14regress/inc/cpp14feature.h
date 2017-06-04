#ifndef CPP14REGRESS_CPP_14_FEATURE_H
#define CPP14REGRESS_CPP_14_FEATURE_H

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
#include "utils.h"

namespace cpp14regress {
    class FeatureVisitor : public clang::RecursiveASTVisitor<FeatureVisitor> {
    protected:
        clang::CompilerInstance *f_compilerInstance;
        clang::Rewriter *f_rewriter;
        clang::ASTContext *f_astContext;
        clang::SourceManager *f_sourceManager;
        clang::LangOptions *f_langOptions;
        clang::Preprocessor *f_preprocessor;


        virtual void endSourceFileAction() {}

        virtual void beginSourceFileAction() {}

    public:
        FeatureVisitor(clang::CompilerInstance *ci);

        virtual void EndSourceFileAction();

        virtual void BeginSourceFileAction();

        virtual features::type type() = 0;

        virtual bool VisitVarDecl(clang::VarDecl *) { return true; }

        virtual bool VisitLambdaExpr(clang::LambdaExpr *) { return true; }

        virtual bool VisitCXXForRangeStmt(clang::CXXForRangeStmt *) { return true; }

        virtual bool VisitEnumDecl(clang::EnumDecl *) { return true; }

        virtual bool VisitTypeLoc(clang::TypeLoc) { return true; }

        virtual bool VisitUserDefinedLiteral(clang::UserDefinedLiteral *) { return true; }

        virtual bool VisitFunctionDecl(clang::FunctionDecl *) { return true; }

        virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *) { return true; }

        virtual bool VisitCXXMethodDecl(clang::CXXMethodDecl *) { return true; }

        virtual bool VisitTypeAliasDecl(clang::TypeAliasDecl *) { return true; }

        virtual bool VisitCXXConversionDecl(clang::CXXConversionDecl *) { return true; }

        virtual bool VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *) { return true; }

        virtual bool VisitIntegerLiteral(clang::IntegerLiteral *) { return true; }

        virtual bool VisitFloatingLiteral(clang::FloatingLiteral *) { return true; }

        virtual bool VisitCXXConstructExpr(clang::CXXConstructExpr *) { return true; }

        virtual bool VisitDecltypeTypeLoc(clang::DecltypeTypeLoc) { return true; }

        virtual bool VisitStringLiteral(clang::StringLiteral *) { return true; }

        virtual bool VisitCXXStdInitializerListExpr(clang::CXXStdInitializerListExpr *) { return true; }

        virtual bool VisitStaticAssertDecl(clang::StaticAssertDecl *) { return true; }

        virtual bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl *) { return true; }

        virtual bool VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr *nullPtrExpr) { return true; }

        virtual bool VisitAttr(clang::Attr *) { return true; }

        virtual bool VisitVarTemplateDecl(clang::VarTemplateDecl *) { return true; }

        virtual bool VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *) { return true; }

        virtual bool VisitCXXNoexceptExpr(clang::CXXNoexceptExpr *) { return true; }

        virtual bool VisitTemplateDecl(clang::TemplateDecl *) { return true; }

        virtual bool VisitPackExpansionTypeLoc(clang::PackExpansionTypeLoc) { return true; }

        virtual ~FeatureVisitor() { delete f_rewriter; }
    };
}

#endif /*CPP14REGRESS_CPP_14_FEATURE_H*/