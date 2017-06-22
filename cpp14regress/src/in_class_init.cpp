
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

#include "in_class_init.h"
#include "utils.h"
#include "ast_to_dot.h"

#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;


    bool MemberInitReplacer::VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl) {
        if (!fromUserFile(recordDecl, f_sourceManager))
            return true;

        vector<const CXXConstructorDecl *> definitions;

        for (const CXXConstructorDecl *declaration : recordDecl->ctors()) {
            if (declaration->isUserProvided()) {
                const FunctionDecl *fd = nullptr;
                declaration->isDefined(fd);
                if (auto definition = dyn_cast_or_null<CXXConstructorDecl>(fd))
                    definitions.push_back(definition);
            }
        }

        bool canRegress = !recordDecl->getNameAsString().empty();
        canRegress &= !definitions.empty();
        replacement::result res = (canRegress) ? replacement::result::removed :
                                  replacement::result::found;

        bool needRegress = false;
        for (const FieldDecl *field : recordDecl->fields()) {
            //cout << "Field of " << recordDecl->getNameAsString() << ": "
            //     << toString(field, f_astContext) << " -- "
            //     << field->hasInClassInitializer() << endl;
            if (field->hasInClassInitializer()) {
                SourceRange initRange = field->getInClassInitializer()->getSourceRange();
                initRange.setBegin(findTokenBeginAfterLoc(
                        field->getLocation(), tok::TokenKind::equal, 1, f_astContext));
                if (canRegress) {
                    f_rewriter->InsertTextBefore(initRange.getBegin(), Comment::block::begin());
                    f_rewriter->InsertTextAfterToken(initRange.getEnd(), Comment::block::end());
                }
                f_rewriter->InsertTextBefore(field->getLocStart(), Comment::line(
                        replacement::info(type(), res)) + "\n");
                needRegress = true;
            }
        }
        if (!canRegress || !needRegress)
            return true;

        for (const CXXConstructorDecl *definition : definitions) {
            //cout << "Definition: " << toString(definition, f_astContext) << " -- "
            //     << definition->getLocStart().printToString(*f_sourceManager) << endl;

            auto last = find_if(definition->init_rbegin(), definition->init_rend(),
                                [](CXXCtorInitializer *init) {
                                    return (*init).isWritten();
                                });
            SourceLocation initBegin = findTokenEndAfterLoc(definition->getLocation(),
                                                            tok::TokenKind::r_paren, f_astContext);
            SourceLocation initEnd = (last == definition->init_rend()) ? initBegin :
                                     definition->getBody()->getLocStart().getLocWithOffset(-1);
            bool replace = false;
            vector<string> inits;
            for (auto init = definition->init_begin(); init != definition->init_end(); init++) {
                //cout << "Init of " << recordDecl->getNameAsString() << ": "
                //     << toString((*init)->getSourceRange(), f_astContext) << endl;
                if ((*init)->isWritten()) {
                    inits.push_back(
                            toString((*init)->getSourceRange(), f_astContext)); //TODO fix toString
                } else if ((*init)->isInClassMemberInitializer()) {
                    if (auto field = (*init)->getMember()) {
                        //SourceRange initRange = field->getInClassInitializer()->getSourceRange();
                        //initRange.setBegin(findTokenEndAfterLoc(
                        //        field->getLocation(), tok::TokenKind::equal, f_astContext, true));
                        SourceRange initRange = (*init)->getSourceRange();
                        string initVal;
                        if (initRange.isValid())
                            initVal = toString(initRange, f_astContext);
                        else
                            initVal = Comment::block(
                                    replacement::info(type(), replacement::result::error));
                        string init(field->getNameAsString() + "(" + initVal + ")");
                        inits.push_back(init);
                        replace = true;
                    }
                }
            }
            if (replace) {
                string initCode = " : ";
                for (auto init = inits.begin();;) {
                    initCode += *init;
                    if (++init != inits.end())
                        initCode += ", ";
                    else break;
                }
                f_rewriter->ReplaceText(SourceRange(initBegin, initEnd), initCode);
                f_rewriter->InsertTextBefore(definition->getLocStart(), Comment::line(
                        replacement::info(type(), replacement::result::replaced)) + "\n");
            }
        }
        /*if (!ctorFound) {
            string initCtor = string("public:\n" + recordDecl->getNameAsString() + "() : ");
            vector <string> inits;
            for (auto field = recordDecl->field_begin(); field != recordDecl->field_end(); field++) {
                if (auto init = field->getInClassInitializer()) {
                    SourceRange initRange = init->getSourceRange();
                    initRange.setBegin(findTokenEndAfterLoc(
                            field->getLocation(), tok::TokenKind::equal, f_astContext, true));
                    string initVal;
                    if (initRange.isValid())
                        initVal = toString(initRange, f_astContext);
                    else
                        initVal = Comment::block(
                                replacement::info(type(), replacement::result::error));
                    inits.push_back(string(field->getNameAsString() +
                                           "(" + initVal + ")"));
                }
            }
            for (auto it = inits.begin(); it != inits.end();) {
                initCtor += *it;
                if (++it != inits.end())
                    initCtor += ", ";
            }
            initCtor += " {}\n";
            f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), Comment::line(
                    replacement::end(type(), replacement::result::inserted)) + "\n");
            f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), initCtor);
            f_rewriter->InsertTextBefore(recordDecl->getRBraceLoc(), Comment::line(
                    replacement::begin(type(), replacement::result::inserted)) + "\n");
            return true;
        }*/
        return true;
    }
}
