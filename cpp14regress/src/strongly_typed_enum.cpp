
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
        if (!fromUserFile(enumDecl, f_sourceManager))
            return true;

        SourceRange typeRange = enumDecl->getIntegerTypeRange();
        if (typeRange.isValid()) {
            typeRange.setEnd(Lexer::getLocForEndOfToken(typeRange.getBegin(), 0,
                                                        *f_sourceManager, *f_langOptions));
            typeRange.setBegin(Lexer::getLocForEndOfToken(enumDecl->getLocation(), 0,
                                                          *f_sourceManager, *f_langOptions));
            typeRange.setBegin(findTokenLoc(typeRange, *f_astContext, tok::TokenKind::colon, 1));

            if (typeRange.isValid()) { //TODO change for commentator class
                //rewriter()->InsertText(typeRange.getBegin(), "/*");
                //rewriter()->InsertText(typeRange.getEnd(), "*/");
            }
        }
        return true;
    }

    bool ImprovedEnumReplacer::VisitTypeLoc(clang::TypeLoc typeLoc) {
        if (!fromUserFile(&typeLoc, f_sourceManager))
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
                                                                              0, *f_sourceManager,
                                                                              *f_langOptions);
                        //TODO doesn't work
                        if (Lexer::findLocationAfterToken(insertLoc, tok::TokenKind::coloncolon,
                                                          *f_sourceManager, *f_langOptions,
                                                          true).isInvalid()) {
                            cout << insertLoc.printToString(*f_sourceManager) << endl;
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
                                                             *f_sourceManager, *f_langOptions));
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
}