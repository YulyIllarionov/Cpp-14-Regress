
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

#include "lambda_function.h"
#include "utils.h"

#include <iostream>
#include <string>

namespace cpp14regress {

    using namespace std;
    using namespace clang;

    LambdaFunctionReplacer::LambdaFunctionReplacer(ASTContext *context)
    {
        f_context = context;
        f_rewriter = new Rewriter(context->getSourceManager(),
                                  context->getLangOpts());
    }

    bool LambdaFunctionReplacer::VisitLambdaExpr(LambdaExpr *lambda) {

        static int count = 0;
        cout << "Lambda " << count << ": " << endl << "----------------" << endl;
        //cout << stringFromStmt(lambda, f_context) << endl << endl;

        for (LambdaCapture capture : lambda->captures())
        {
            VarDecl* captured_var = capture.getCapturedVar();
            cout << captured_var->getType().getAsString() << " "
                 << captured_var->getQualifiedNameAsString() << endl;
        }

        //cout << stringFromStmt(lambda->getBody(), f_context) << endl << endl;

        cout  << "----------------" << endl;
        count++;
        return true;
    }

}