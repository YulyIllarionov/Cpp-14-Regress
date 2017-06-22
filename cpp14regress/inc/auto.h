#ifndef CPP14REGRESS_AUTO_H
#define CPP14REGRESS_AUTO_H

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
#include "utils.h"

#include<set>

namespace cpp14regress {

    class AutoReplacer : public FeatureVisitor { //TODO fix
    public:

        AutoReplacer(clang::CompilerInstance *ci) : FeatureVisitor(ci) {}

        virtual features::type type() { return features::type::auto_keyword; }

        //virtual bool VisitAutoTypeLoc(clang::AutoTypeLoc typeLoc);

        virtual bool VisitVarDecl(clang::VarDecl *varDecl);

        virtual bool VisitFunctionDecl(clang::FunctionDecl *funDecl);

        virtual bool VisitDeclStmt(clang::DeclStmt *declStmt);

    private:
        std::set<clang::Decl *> f_multipleDecl;
        bool f_firstInMultiple = true;
    };
}

#endif /*CPP14REGRESS_AUTO_H*/
