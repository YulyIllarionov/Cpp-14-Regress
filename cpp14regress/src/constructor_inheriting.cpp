
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

#include "constructor_inheriting.h"
#include "utils.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool ConstructorInheritingReplacer::VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl) {
        if (!fromUserFile(recordDecl, f_sourceManager))
            return true;

        bool inherit = false;
        for (auto it = recordDecl->decls_begin(); it != recordDecl->decls_end(); it++)
            if (auto usingDecl = dyn_cast_or_null<UsingDecl>(*it))
                if (dyn_cast_or_null<CXXConstructorDecl>(usingDecl->getNextDeclInContext())) {
                    f_rewriter->InsertTextBefore(usingDecl->getLocStart(),
                                                 Comment::block::begin() +
                                                 replacement::info(type(), replacement::result::removed) + " ");
                    f_rewriter->InsertTextAfterToken(usingDecl->getLocEnd(), Comment::block::end());
                    inherit = true;
                    break;
                }
        string status;
        if (inherit) {
            std::vector<string> ctorDecls;
            for (CXXConstructorDecl *ctor : recordDecl->ctors()) {
                if (auto baseCtor = ctor->getInheritedConstructor()) {
                    //cout << "Base: " << toString(baseCtor, f_astContext) << endl;
                    //Constructor decl
                    string ctorDecl(toString(SourceRange(baseCtor->getLocStart(), baseCtor->getLocation()),
                                             f_astContext, false));
                    ctorDecl += ctor->getNameAsString() + "(";
                    //for (auto arg : ctor->params()) {
                    //    cout << arg->getType().getAsString(PrintingPolicy(*f_langOptions)) << " "
                    //         << arg->getNameAsString() << ", ";
                    //}
                    //cout << endl;
                    ArgNameGenerator::reset();
                    for (auto it = ctor->param_begin();;) {
                        ctorDecl += (*it)->getType().getAsString(PrintingPolicy(*f_langOptions)) + " ";
                        if ((*it)->getNameAsString().empty()) {
                            ctorDecl += string(" " + ArgNameGenerator::toString());
                            ArgNameGenerator::generate();
                        } else {
                            ctorDecl += (*it)->getNameAsString();
                        }
                        if (++it != ctor->param_end())
                            ctorDecl += ", ";
                        else
                            break;
                    }
                    ctorDecl += string(") : " + baseCtor->getNameAsString() + "(");
                    //init call
                    ArgNameGenerator::reset();
                    /*for (auto init = ctor->init_begin(); init != ctor->init_end(); init++) {
                        if (auto ctorInit = dyn_cast_or_null<CXXConstructExpr>((*init)->getInit())) {
                            for (auto arg = ctorInit->arg_begin(); arg != ctorInit->arg_end(); arg++) {
                                cout << (*arg)->getStmtClassName() << endl;
                            }
                        }
                    }*/
                    for (auto it = ctor->param_begin();;) {
                        string argName;
                        if ((*it)->getNameAsString().empty()) {
                            argName = ArgNameGenerator::toString();
                            ArgNameGenerator::generate();
                        } else {
                            argName += (*it)->getNameAsString();
                        }
                        //if ((*it)->getType()->isRValueReferenceType()) {
                        //    argName.insert(0, "std::move(");
                        //    argName += ')';
                        //}
                        ctorDecl += argName;
                        if (++it != ctor->param_end())
                            ctorDecl += ", ";
                        else
                            break;
                    }
                    ctorDecl += ") {}";
                    ctorDecls.push_back(ctorDecl);

                    /*if (declBeginLoc.isValid() && nameBeginLoc.isValid() &&
                        nameEndLoc.isValid() && rParenLoc.isValid()) {
                        const char *declBegin = f_sourceManager->getCharacterData(declBeginLoc);
                        const char *nameBegin = f_sourceManager->getCharacterData(nameBeginLoc);
                        const char *nameEnd = f_sourceManager->getCharacterData(nameEndLoc);
                        const char *declEnd = f_sourceManager->getCharacterData(rParenLoc);

                        string ctorDecl = string(declBegin, declEnd);
                        ctorDecl.replace(nameBegin - declBegin, nameEnd - nameBegin,
                                         recordDecl->getNameAsString());
                        ctorDecl += string(" : " + baseCtor->getNameAsString() + "(");
                        for (auto it = baseCtor->param_begin();;) {
                            ctorDecl += (*it)->getNameAsString();
                            if (++it != baseCtor->param_end())
                                ctorDecl += ", ";
                            else
                                break;
                        }
                        ctorDecl += ") {}";
                        ctorDecls.push_back(ctorDecl);
                        cout << "New ctor: " << ctorDecl << endl;
                    }*/
                }
            }
            if (!ctorDecls.empty()) {
                f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), Comment::line(
                        replacement::end(type(), replacement::result::inserted) + status) + "\n");
                for (auto ctorDecl : ctorDecls) {
                    f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), ctorDecl + "\n");
                }
                f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), "public: \n");
                f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), Comment::line(
                        replacement::begin(type(), replacement::result::inserted) + status) + "\n");
            }
        }
        return true;
    }

}
