

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
#include "llvm/ADT/DenseMap.h"

#include "default.h"
#include "base_types.h"
#include "utils.h"
#include "ast_to_dot.h"

#include <iostream>
#include <string>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    DefaultKeywordReplacer::DefaultKeywordReplacer(ASTContext *context, cpp14features_stat *stat) :
            f_context(context), f_stat(stat) {
        f_rewriter = new Rewriter(context->getSourceManager(),
                                  context->getLangOpts());
    }


    //TODO comment move constructor operator=
    //TODO check in class non constant initialization in c++03
    bool DefaultKeywordReplacer::VisitCXXMethodDecl(clang::CXXMethodDecl *methodDecl) {
        if (!inProcessedFile(methodDecl, f_context))
            return true;

        if (methodDecl->isExplicitlyDefaulted()) {
            SourceLocation defaultBegin = methodDecl->getLocStart().getLocWithOffset(
                    toString(methodDecl, f_context).find_last_of(')') + 1); //TODO fix
            SourceLocation defaultEnd = methodDecl->getLocEnd();
            string defaultBody;
            CXXRecordDecl *baseClass = methodDecl->getParent();

            if (auto constructorDecl = dyn_cast<CXXConstructorDecl>(methodDecl)) {
                if (constructorDecl->isDefaultConstructor()) {
                    //cout << "Defaulted default constructor: " << toString(constructorDecl, f_context) << endl << "---------" << endl;
                    defaultBody = " {}";
                } else if (constructorDecl->isCopyConstructor()) {
                    //cout << "Defaulted copy constructor: " << toString(constructorDecl, f_context) << endl << "---------"<< endl;
                    defaultBody = " {\n";
                    for (auto it = baseClass->field_begin(); it != baseClass->field_end(); it++) {
                        string fieldCopy = "this->" + (*it)->getNameAsString() + " = " +
                                           methodDecl->parameters().front()->getNameAsString()
                                           + "." + (*it)->getNameAsString() + ";\n";
                        defaultBody += fieldCopy;
                    }
                    defaultBody += "}\n";
                } else if (constructorDecl->isMoveConstructor()) {
                    //cout << "Defaulted move constructor: " << toString(constructorDecl, f_context) << endl << "---------" << endl;
                    //TODO
                }
            } else if (isa<CXXDestructorDecl>(methodDecl)) {
                //cout << "Defaulted destructor: " << toString(destructorDecl, f_context) << endl << "---------" << endl;
                defaultBody = " {}";
            } else {
                if (methodDecl->isCopyAssignmentOperator()) {
                    //cout << "Defaulted copy operator: " << toString(methodDecl, f_context) << endl << "---------" << endl;
                    defaultBody = " {\n"; //TODO unnecessary duplication
                    for (auto it = baseClass->field_begin(); it != baseClass->field_end(); it++) {
                        string fieldCopy = "this->" + (*it)->getNameAsString() + " = " +
                                           methodDecl->parameters().front()->getNameAsString()
                                           + "." + (*it)->getNameAsString() + ";\n";
                        defaultBody += fieldCopy;
                    }
                    defaultBody += "}\n";
                } else if (methodDecl->isMoveAssignmentOperator()) {
                    //cout << "Defaulted move operator: " << toString(methodDecl, f_context) << endl << "---------" << endl;
                    //TODO
                }
            }
            f_rewriter->RemoveText(SourceRange(defaultBegin, defaultEnd));
            f_rewriter->InsertText(defaultBegin, defaultBody, true, true);
        }
        return true;
    }
}