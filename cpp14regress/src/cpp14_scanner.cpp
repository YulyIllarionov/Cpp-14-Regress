
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
#include "clang/AST/DeclCXX.h"
#include "llvm/ADT/DenseMap.h"

#include <algorithm>

#include "cpp14_scanner.h"
#include "base_types.h"
#include "utils.h"


namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool Cpp14scanner::VisitCXXForRangeStmt(CXXForRangeStmt *forLoop) {
        if (!inProcessedFile(forLoop, f_context))
            return true;
        f_stat->push(cpp14features::range_based_for, forLoop->getLocStart());
        return true;
    }

    bool Cpp14scanner::VisitLambdaExpr(clang::LambdaExpr *lambdaExpr) {
        if (!inProcessedFile(lambdaExpr, f_context))
            return true;
        f_stat->push(cpp14features::lambda_function, lambdaExpr->getLocStart());
        return true;
    }

    bool Cpp14scanner::VisitDeclaratorDecl(clang::DeclaratorDecl *valueDecl) {
        if (!inProcessedFile(valueDecl, f_context))
            return true;
        if (isa<AutoType>(valueDecl->getType().getTypePtr())) {
            f_stat->push(cpp14features::auto_keyword, valueDecl->getLocStart());
        } else if (isa<DecltypeType>(valueDecl->getType().getTypePtr())) {
            f_stat->push(cpp14features::decltype_keyword, valueDecl->getLocStart());
            //cout << "decltype: " << valueDecl->getLocStart().printToString(f_context->getSourceManager()) << endl;
        }
        return true;
    }

    bool Cpp14scanner::VisitFunctionDecl(clang::FunctionDecl *functionDecl) {
        if (!inProcessedFile(functionDecl, f_context))
            return true;
        if (functionDecl->isConstexpr())
            f_stat->push(cpp14features::constexpr_keyword, functionDecl->getLocStart());
        if (functionDecl->getNameAsString().find("\"\"") != string::npos) { //TODO fix
            //cout << "User-defined literal: " << toSting(functionDecl, f_context) << endl;
            f_stat->push(cpp14features::user_defined_literals, functionDecl->getLocStart());
        }
        if (functionDecl->getType().getAsString().find("->") != string::npos) //TODO need fix
        {
            //cout << "Alternate syntax: " << functionDecl->getLocStart().printToString(f_context->getSourceManager()) << endl;
            f_stat->push(cpp14features::alternative_function_syntax, functionDecl->getLocStart());
        }
        return true;
    }

    bool Cpp14scanner::VisitCXXMethodDecl(clang::CXXMethodDecl *methodDecl) {
        if (!inProcessedFile(methodDecl, f_context))
            return true;
        string methodString = toSting(methodDecl, f_context);
        if (methodDecl->isExplicitlyDefaulted())
            f_stat->push(cpp14features::default_specifier, methodDecl->getLocStart());
        if (methodDecl->isDeleted())
            f_stat->push(cpp14features::delete_specifier, methodDecl->getLocStart());
        if (methodString.find(" override") != string::npos) //TODO
            f_stat->push(cpp14features::override_specifier, methodDecl->getLocStart());
        if (methodString.find(" final") != string::npos) //TODO
            f_stat->push(cpp14features::final_specifier, methodDecl->getLocStart());
        return true;
    }

    bool Cpp14scanner::VisitCXXConversionDecl(clang::CXXConversionDecl *conversionMethod) {
        if (!inProcessedFile(conversionMethod, f_context))
            return true;
        if (conversionMethod->isExplicitSpecified())
            f_stat->push(cpp14features::explicit_specifier, conversionMethod->getLocStart());
        return true;
    }

    bool Cpp14scanner::VisitCXXConstructorDecl(clang::CXXConstructorDecl *constructorDecl) {
        if (!inProcessedFile(constructorDecl, f_context))
            return true;
        if (constructorDecl->isExplicitSpecified())
            f_stat->push(cpp14features::explicit_specifier, constructorDecl->getLocStart());
        if (constructorDecl->isDelegatingConstructor())
            f_stat->push(cpp14features::constuctor_delegation, constructorDecl->getLocStart());
        return true;
    }

    bool Cpp14scanner::VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr *nullPtrExpr) {
        if (!inProcessedFile(nullPtrExpr, f_context))
            return true;
        //cout << "Nullptr: " << toSting(nullPtrExpr, f_context) << " -- "
        //     << nullPtrExpr->getLocStart().printToString(f_context->getSourceManager()) << endl;
        f_stat->push(cpp14features::null_pointer_constant, nullPtrExpr->getLocStart());
        return true;
    }

    bool Cpp14scanner::VisitEnumDecl(clang::EnumDecl *enumDecl) {
        if (!inProcessedFile(enumDecl, f_context))
            return true;
        if (enumDecl->isScopedUsingClassTag())
            f_stat->push(cpp14features::enum_class, enumDecl->getLocStart());
        return true;
    }

    bool Cpp14scanner::VisitRecordDecl(clang::RecordDecl *recordDecl) {
        if (!inProcessedFile(recordDecl, f_context))
            return true;
        //cout << "Record decl" << endl << "---------" << endl
        //     << toSting(recordDecl, f_context) << "---------" << endl;
        if (recordDecl->isUnion()) {
            //cout << "Union: " << toSting(recordDecl, f_context) << endl;
            for (auto it = recordDecl->field_begin(); it != recordDecl->field_end(); it++) {
                if (!((*it)->getType().isCXX98PODType(*f_context))) {
                    //cout << "Unrestricted union: " << toSting(recordDecl, f_context) << endl;
                    f_stat->push(cpp14features::unrestricted_unions, recordDecl->getLocStart());
                    break;
                }
            }
        }
        for (auto it = recordDecl->attr_begin(); it != recordDecl->attr_end(); it++) {
            if ((*it)->getKind() == attr::Kind::Aligned) {
                //cout << "Attr: " << toSting(recordDecl, f_context) << endl;
                f_stat->push(cpp14features::alignas_specifier, recordDecl->getLocStart());
            }
        }
        return true;
    }

    bool Cpp14scanner::VisitStringLiteral(clang::StringLiteral *literal) {
        if (!inProcessedFile(literal, f_context))
            return true;
        string s = toSting(literal, f_context);
        if (s[max(int(s.find('\"')) - 1, 0)] == 'R') { //TODO need fix
            //cout << "Raw string literal: " << toSting(literal, f_context) << endl;
            f_stat->push(cpp14features::raw_string_literals, literal->getLocStart());
        }
        if ((literal->isUTF8()) ||
            (literal->isUTF16()) ||
            (literal->isUTF32()))
            f_stat->push(cpp14features::unicode_string_literals, literal->getLocStart());
        return true;
    }

    //TODO fix, exception string too long
    bool Cpp14scanner::VisitIntegerLiteral(clang::IntegerLiteral *literal) {
        if (!inProcessedFile(literal, f_context))
            return true;
        string i = toSting(literal, f_context);
        //cout << "Integer literal: " << i << endl;
        if (i.find('\'') != string::npos) { //TODO need fix
            f_stat->push(cpp14features::digit_separators, literal->getLocStart());
            //cout << "Integer with separators: " << i << endl;
        }
        return true;
    }
    //TODO fix, exception string too long
    bool Cpp14scanner::VisitFloatingLiteral(clang::FloatingLiteral *literal) {
        if (!inProcessedFile(literal, f_context))
            return true;
        string f = toSting(literal, f_context);
        if (f.find('\'') != string::npos) { //TODO need fix
            f_stat->push(cpp14features::digit_separators, literal->getLocStart());
            //cout << "Float with separators: " << f << endl;
        }
        return true;
    }

    bool Cpp14scanner::VisitCXXNoexceptExpr(clang::CXXNoexceptExpr *noexceptExpr) {
        if (!inProcessedFile(noexceptExpr, f_context))
            return true;
        f_stat->push(cpp14features::noexcept_keyword, noexceptExpr->getLocStart());
        return true;
    }

    bool Cpp14scanner::VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *aliasTemplateDecl) {
        if (!inProcessedFile(aliasTemplateDecl, f_context))
            return true;
        f_stat->push(cpp14features::alias_template, aliasTemplateDecl->getLocStart()); //TODO
        //cout << "alias template: " << toSting(aliasTemplateDecl, f_context) << endl;
        return true;
    }

    bool Cpp14scanner::VisitTypeAliasDecl(clang::TypeAliasDecl *aliasTypeDecl) {
        if (!inProcessedFile(aliasTypeDecl, f_context))
            return true;
        if (isa<TypeAliasTemplateDecl>(aliasTypeDecl)) {
            f_stat->push(cpp14features::alias_template, aliasTypeDecl->getLocStart()); //TODO
            //cout << "alias template: " << toSting(aliasTypeDecl, f_context) << endl;
        } else {
            f_stat->push(cpp14features::alias_type, aliasTypeDecl->getLocStart());
            //cout << "alias type: " << toSting(aliasTypeDecl, f_context) << endl;
        }
        return true;
    }

    bool Cpp14scanner::


    VisitSizeOfPackExpr(clang::SizeOfPackExpr *sizeofPackExpr) {
        if (!inProcessedFile(sizeofPackExpr, f_context))
            return true;
        cout << "Found sizeof...: " << toSting(sizeofPackExpr, f_context) << endl;
        return true;
    }

    bool Cpp14scanner::VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *sizeofOrAlignof) {
        if (!inProcessedFile(sizeofOrAlignof, f_context))
            return true;
        if (sizeofOrAlignof->getKind() == UnaryExprOrTypeTrait::UETT_SizeOf) {
            if (!sizeofOrAlignof->isArgumentType()) {

                Expr *arg = sizeofOrAlignof->getArgumentExpr();
                while (auto parenExpr = dyn_cast<ParenExpr>(arg))
                    arg = parenExpr->getSubExpr();

                if (auto dre = dyn_cast<DeclRefExpr>(arg)) {
                    if (dre->hasQualifier()) { //TODO check
                        if ((dre->getQualifier()->getKind() ==
                             clang::NestedNameSpecifier::SpecifierKind::TypeSpec) ||
                            (dre->getQualifier()->getKind() ==
                             clang::NestedNameSpecifier::SpecifierKind::TypeSpecWithTemplate)) {
                            //cout << "sizeof: " << toSting(sizeofOrAlignof->getArgumentExpr(), f_context)
                            //     << " is implict" << endl;
                            f_stat->push(cpp14features::implict_sizeof, sizeofOrAlignof->getLocStart());
                        }
                    }
                }
            }
        } else if (sizeofOrAlignof->getKind() == UnaryExprOrTypeTrait::UETT_AlignOf) {
            f_stat->push(cpp14features::alignof_operator, sizeofOrAlignof->getLocStart());
            //cout << "alignof: " << toSting(sizeofOrAlignof, f_context) << endl;
        }
        return true;
    }

    bool Cpp14scanner::VisitRedeclarableTemplateDecl(clang::RedeclarableTemplateDecl *templateDecl) {
        if (!inProcessedFile(templateDecl, f_context))
            return true;
        TemplateParameterList *tps = templateDecl->getTemplateParameters();
        for (auto it = tps->begin(); it != tps->end(); it++) {
            if (auto tp = dyn_cast_or_null<TemplateTypeParmDecl>(*it)) {
                if (tp->isParameterPack()) {
                    //cout << "Variadic template: " << toSting(templateDecl, f_context) << endl;
                    f_stat->push(cpp14features::variadic_templates, templateDecl->getLocStart());
                    break;
                }
            }
        }
        //if (expr->containsUnexpandedParameterPack()) { //TODO may also works

        return true;
    }

    bool Cpp14scanner::VisitVarDecl(clang::VarDecl *varDecl) {
        if (!inProcessedFile(varDecl, f_context))
            return true;
        if (auto *bt = dyn_cast<BuiltinType>(varDecl->getType())) {
            if ((bt->getKind() == BuiltinType::LongLong) ||
                (bt->getKind() == BuiltinType::ULongLong)) {
                //cout << "Long long int: " << toSting(varDecl, f_context) << " -- "
                //     << varDecl->getLocStart().printToString(f_context->getSourceManager()) << endl;
                f_stat->push(cpp14features::long_long_int, varDecl->getLocStart());
            }
        }
        //if (varDecl->getInitStyle() == VarDecl::InitializationStyle::ListInit) {
        //    //cout << "Uniform init: " << toSting(varDecl, f_context) << endl;
        //    f_stat->push(cpp14features::uniform_initialization, varDecl->getLocStart());
        //}
        if (varDecl->hasInit()) {
            if (isa<InitListExpr>(varDecl->getInit())) {
                cout << "Uniform init: " << toSting(varDecl, f_context) << endl;
                f_stat->push(cpp14features::uniform_initialization, varDecl->getLocStart());
            }
        }
        return true;
    }

    bool Cpp14scanner::VisitCStyleCastExpr(clang::CStyleCastExpr *castExpr) {
        if (!inProcessedFile(castExpr, f_context))
            return true;
        if (auto *bt = dyn_cast<BuiltinType>(castExpr->getTypeAsWritten())) {
            if ((bt->getKind() == BuiltinType::LongLong) ||
                (bt->getKind() == BuiltinType::ULongLong)) {
                //cout << "Cast kind: " << castExpr->getCastKindName() << " -- "
                //     << castExpr->getLocStart().printToString(f_context->getSourceManager()) << endl;
                f_stat->push(cpp14features::long_long_int, castExpr->getLocStart());
            }
        }
        return true;
    }

    bool Cpp14scanner::VisitCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr *castExpr) {
        if (!inProcessedFile(castExpr, f_context))
            return true;
        if (auto *bt = dyn_cast<BuiltinType>(castExpr->getTypeAsWritten())) {
            if ((bt->getKind() == BuiltinType::LongLong) ||
                (bt->getKind() == BuiltinType::ULongLong)) {
                //cout << "Cast kind: " << castExpr->getCastKindName() << " -- "
                //     << castExpr->getLocStart().printToString(f_context->getSourceManager()) << endl;
                f_stat->push(cpp14features::long_long_int, castExpr->getLocStart());
            }
        }
        Expr *arg = castExpr->getSubExpr();
        while (auto parenExpr = dyn_cast<ParenExpr>(arg))
            arg = parenExpr->getSubExpr();
        if (isa<InitListExpr>(arg)) {
            cout << "Uniform cast: " << toSting(castExpr, f_context) << endl;
            f_stat->push(cpp14features::uniform_initialization, castExpr->getSubExpr()->getLocStart());
        }
        return true;
    }

    bool Cpp14scanner::VisitVarTemplateDecl(clang::VarTemplateDecl *varTemplate) {
        if (!inProcessedFile(varTemplate, f_context))
            return true;
        f_stat->push(cpp14features::variable_templates, varTemplate->getLocStart());
        //cout << "Variable template: " << toSting(varTemplate, f_context) << endl;
        return true;
    }

    bool Cpp14scanner::VisitValueDecl(clang::ValueDecl *valueDecl) {
        if (!inProcessedFile(valueDecl, f_context))
            return true;
        for (auto it = valueDecl->attr_begin(); it != valueDecl->attr_end(); it++) {
            if ((*it)->getKind() == attr::Kind::Aligned) {
                //cout << "Attr: " << toSting(valueDecl, f_context) << endl;
                f_stat->push(cpp14features::alignas_specifier, valueDecl->getLocStart());
            }
        }
        return true;
    }

    //bool Cpp14scanner::VisitUserDefinedLiteral(clang::UserDefinedLiteral *userDefinedLiteral) {
    //    if (!inProcessedFile(userDefinedLiteral, f_context))
    //        return true;
    //    //cout << "User-defined literal: " << toSting(userDefinedLiteral, f_context) << endl;
    //    //if (FunctionDecl *func = userDefinedLiteral->getDirectCallee())
    //    //    cout << func->getNameAsString() << endl;
    //    return true;
    //}

    //bool Cpp14scanner::VisitAttr(clang::Attr *attr) {
    //    if (attr->getLocation().isValid())
    //        if (f_context->getSourceManager().getFileCharacteristic(attr->getLocation()) !=
    //            clang::SrcMgr::CharacteristicKind::C_User)
    //            return true;
//
    //    if (attr->getKind() == attr::Kind::Aligned)
    //        cout << "Attr: "
    //             << std::string(f_context->getSourceManager().getCharacterData(attr->getRange().getBegin()),
    //                            f_context->getSourceManager().getCharacterData(
    //                                    SourceLocation(Lexer::getLocForEndOfToken(
    //                                            attr->getRange().getEnd(), 0, f_context->getSourceManager(),
    //                                            f_context->getLangOpts()))) -
    //                            f_context->getSourceManager().getCharacterData(attr->getRange().getBegin()))
    //             << endl;
//
//
    //    return true;
    //}

}
