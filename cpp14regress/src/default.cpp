

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
#include "cpp14feature.h"
#include "utils.h"
#include "ast_to_dot.h"

#include <iostream>
#include <string>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    //TODO comment move constructor operator=
    //TODO check in class non constant initialization in c++03
    bool DefaultReplacer::VisitCXXMethodDecl(clang::CXXMethodDecl *methodDecl) {
        if (!fromUserFile(methodDecl, f_sourceManager))
            return true;

        if (methodDecl->isExplicitlyDefaulted()) {
            SourceRange rewriteRange;
            rewriteRange.setBegin(findTokenEndAfterLoc(methodDecl->getLocation(),
                                                       tok::TokenKind::r_paren, f_astContext));
            rewriteRange.setEnd(findTokenEndAfterLoc(methodDecl->getLocEnd(),
                                                     tok::TokenKind::semi, f_astContext));
            replacement::result res = replacement::result::found;
            if (rewriteRange.isValid()) {
                string defaultBody;
                string paramName;
                if (!methodDecl->parameters().empty()) {
                    auto param = *methodDecl->param_begin();
                    paramName = param->getNameAsString();
                    if (paramName.empty()) {
                        paramName = "other";
                        f_rewriter->InsertTextAfter(param->getLocation(), paramName);
                    }
                }
                const CXXRecordDecl *baseClass = methodDecl->getParent();
                if (auto constructorDecl = dyn_cast<CXXConstructorDecl>(methodDecl)) {
                    if (constructorDecl->isDefaultConstructor()) {
                        //cout << "Defaulted default constructor: "
                        // << toString(constructorDecl, f_context) << endl << "---------" << endl;
                        defaultBody = " {}";
                    } else if (constructorDecl->isCopyConstructor()) {
                        //cout << "Defaulted copy constructor: "
                        // << toString(constructorDecl, f_context) << endl << "---------"<< endl;
                        if (!baseClass->field_empty())
                            defaultBody = " : ";
                        for (auto it = baseClass->field_begin(); it != baseClass->field_end();) {
                            string fieldCopy = (*it)->getNameAsString() + "(" +
                                               paramName + "." + (*it)->getNameAsString() + ")";
                            if (++it != baseClass->field_end())
                                fieldCopy += ", ";
                            defaultBody += fieldCopy;
                        }
                        defaultBody += " {}";
                    } else if (constructorDecl->isMoveConstructor()) {
                        //cout << "Defaulted move constructor: "
                        // << toString(constructorDecl, f_context) << endl << "---------" << endl;
                        if (!baseClass->field_empty())
                            defaultBody = " : ";
                        for (auto it = baseClass->field_begin(); it != baseClass->field_end();) {
                            string fieldCopy = (*it)->getNameAsString() + "(std::move(" +
                                               paramName + "." + (*it)->getNameAsString() + "))";
                            if (++it != baseClass->field_end())
                                fieldCopy += ", ";
                            defaultBody += fieldCopy;
                        }
                        defaultBody += " {}";
                    }
                } else if (isa<CXXDestructorDecl>(methodDecl)) {
                    //cout << "Defaulted destructor: "
                    // << toString(destructorDecl, f_context) << endl << "---------" << endl;
                    defaultBody = " {}";
                } else {
                    if (methodDecl->isCopyAssignmentOperator()) {
                        //cout << "Defaulted copy operator: "
                        // << toString(methodDecl, f_context) << endl << "---------" << endl;
                        defaultBody = " {\n";
                        for (auto it = baseClass->field_begin(); it != baseClass->field_end(); it++) {
                            string fieldCopy = "this->" + (*it)->getNameAsString() + " = " + paramName
                                               + "." + (*it)->getNameAsString() + ";\n";
                            defaultBody += fieldCopy;
                        }
                        defaultBody += "return *this;\n";
                        defaultBody += "}";
                    } else if (methodDecl->isMoveAssignmentOperator()) {
                        //cout << "Defaulted move operator: "
                        // << toString(methodDecl, f_context) << endl << "---------" << endl;
                        defaultBody = " {\n";
                        for (auto it = baseClass->field_begin(); it != baseClass->field_end(); it++) {
                            string fieldCopy = "this->" + (*it)->getNameAsString() + " = std::move("
                                               + paramName + "." + (*it)->getNameAsString() + ");\n";
                            defaultBody += fieldCopy;
                        }
                        defaultBody += "return *this;\n";
                        defaultBody += "}";
                    }
                }
                f_rewriter->ReplaceText(rewriteRange, defaultBody);
                res = replacement::result::replaced;
            }
            f_rewriter->InsertTextBefore(methodDecl->getLocStart(), Comment::line(
                    replacement::info(type(), res)) + "\n");
        }

        return true;
    }
}