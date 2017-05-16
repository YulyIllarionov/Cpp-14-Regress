
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

#include "strongly_typed_enum.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool ImprovedEnumReplacer::VisitEnumDecl(clang::EnumDecl *enumDecl) { //TODO fix
        if (fromSystemFile(enumDecl, astContext()))
            return true;

        SourceRange typeRange = enumDecl->getIntegerTypeRange();
        if (typeRange.isValid()) {
            typeRange.setEnd(Lexer::getLocForEndOfToken(typeRange.getBegin(), 0,
                                                        sourceManager(), langOptions()));
            typeRange.setBegin(Lexer::getLocForEndOfToken(enumDecl->getLocation(), 0,
                                                          sourceManager(), langOptions()));
            typeRange.setBegin(findTokenLoc(typeRange, astContext(), tok::TokenKind::colon, 1));

            if (typeRange.isValid()) { //TODO change for commentator class
                //rewriter()->InsertText(typeRange.getBegin(), "/*");
                //rewriter()->InsertText(typeRange.getEnd(), "*/");
            }
        }
        return true;
    }

    bool ImprovedEnumReplacer::VisitTypeLoc(clang::TypeLoc typeLoc) {
        if (fromSystemFile(&typeLoc, astContext()))
            return true;
        //if (typeLoc.getLocStart().isInvalid())
        //    return true;
        if (auto type = typeLoc.getTypePtr()) {
            if (EnumDecl *enumDecl = dyn_cast_or_null<EnumDecl>(type->getAsTagDecl())) {
                if (EnumDecl *enumDef = enumDecl->getDefinition()) {
                    if (enumDef->isScopedUsingClassTag()) {
                        //TODO fix type only for ones with definition?
                        if (find(f_enums.begin(), f_enums.end(), enumDef) == f_enums.end()) {
                            f_enums.push_back(enumDef);
                        }
                        //Change enum type
                        SourceLocation insertLoc = Lexer::getLocForEndOfToken(typeLoc.getLocStart(),
                                                                              0, sourceManager(),
                                                                              langOptions());
                        //TODO doesn't work
                        if (Lexer::findLocationAfterToken(insertLoc, tok::TokenKind::coloncolon,
                                                          sourceManager(),
                                                          langOptions(), true).isInvalid()) {
                            cout << insertLoc.printToString(sourceManager()) << endl;
                            //rewriter()->InsertTextAfterToken(typeLoc.getLocStart(),
                            //                                 string("::" + nameForReplace()));
                        }
                    }
                }
            }
        }
        return true;
    }

    void ImprovedEnumReplacer::endSourceFileAction() {
        for (EnumDecl *enumDef : f_enums) {
            //Change enum definition
            SourceRange nameRange(enumDef->getLocStart(),
                                  Lexer::getLocForEndOfToken(enumDef->getLocation(), 0,
                                                             sourceManager(), langOptions()));
            if (nameRange.isInvalid()) {
                //TODO handle error
                return;
            }
            string newName("struct " + enumDef->getNameAsString() + " {\n");
            newName += string("enum " + nameForReplace());
            //rewriter()->ReplaceText(nameRange, newName);
            //rewriter()->InsertText(enumDef->getRBraceLoc(), "};\n");
        }
    }


    /*
    bool StronglyTypedEnumReplacer::VisitValueDecl(ValueDecl *varDecl) {
        if (!inProcessedFile(varDecl, f_context))
            return true;
        if (auto enumType = dyn_cast_or_null<EnumType>(varDecl->getType().getTypePtr())) {
            if (EnumDecl *enumDecl = enumType->getDecl()) {
                cout << "Enum variable: " << toString(varDecl, f_context) << endl;
                f_enumDecls.insert(enumDecl);
            }
        }
        return true;
    }

    bool StronglyTypedEnumReplacer::VisitEnumDecl(EnumDecl *enumDecl) {
        if (!inProcessedFile(enumDecl, f_context))
            return true;

        if (EnumDecl *enumDef = enumDecl->getDefinition()) {
            const SourceManager &sm = f_context->getSourceManager();
            const LangOptions &lo = f_context->getLangOpts();
            //Remove enumeration type
            SourceLocation typeEnd = enumDef->getIntegerTypeRange().getEnd();
            if (typeEnd.isValid()) {
                SourceLocation nameEnd = Lexer::getLocForEndOfToken(enumDef->getLocation(),
                                                                    0, sm, lo);
                //f_rewriter->RemoveText(SourceRange(nameEnd, typeEnd));
            }
            //Remove last comma
            //if (enumDef->enumerator_begin() != enumDef->enumerator_end()) {
            //    auto last = enumDef->enumerator_begin();
            //    while (std::next(last) != enumDef->enumerator_end()) last++;
            //    SourceLocation afterLast = last->getLocEnd(), commaBegin;
            //    Token token;
            //    do {
            //        afterLast = Lexer::AdvanceToTokenCharacter(afterLast, 0, sm, lo);
            //        cout << "End: " << afterLast.printToString(sm) << endl;
            //        if (Lexer::getRawToken(afterLast, token, sm, lo)) {
            //            //TODO not understand
            //            cerr << "Strongly typed enum replacer error" << endl;
            //            break;
            //        }
            //        if (token.is(tok::TokenKind::comma)) {
            //            commaBegin = afterLast;
            //            break;
            //        }
            //    } while (token.isNot(tok::TokenKind::r_brace));
            //    if (commaBegin.isValid()) {
            //        cout << "Last comma:" << commaBegin.printToString(sm) << endl;
            //    }
            //}
        }

        return true;
    }

     */
}