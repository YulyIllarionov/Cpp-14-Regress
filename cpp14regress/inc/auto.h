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

#include "base_types.h"
#include "utils.h"

namespace cpp14regress {

    class AutoReplacer : public FeatureReplacer { //TODO fix
    public:

        AutoReplacer(clang::CompilerInstance *ci) : FeatureReplacer(ci) {}

        virtual cpp14features type() { return cpp14features::auto_keyword; }

        /*virtual bool VisitTypeLoc(clang::TypeLoc typeLoc);*/

        virtual bool VisitVarDecl(clang::VarDecl *varDecl);

        virtual bool VisitFunctionDecl(clang::FunctionDecl *funDecl);

    };
    
}

#endif /*CPP14REGRESS_AUTO_H*/
