
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

    bool Cpp14scanner::VisitExpr(clang::Expr *expr) {
        if (!inProcessedFile(expr, f_context))
            return true;
        //TODO NPC_NeverValueDependent, fix, twice on one file
        if (expr->isNullPointerConstant(*f_context,
                                        Expr::NullPointerConstantValueDependence::NPC_NeverValueDependent)
            == Expr::NullPointerConstantKind::NPCK_CXX11_nullptr) {
            f_stat->push(cpp14features::null_pointer_constant, expr->getLocStart());
            //cout << "nullptr: " << expr->getLocStart().printToString(f_context->getSourceManager()) << endl;
        }
        return true;
    }

    bool Cpp14scanner::VisitEnumDecl(clang::EnumDecl *enumDecl) {
        if (!inProcessedFile(enumDecl, f_context))
            return true;
        if (enumDecl->isScopedUsingClassTag())
            f_stat->push(cpp14features::enum_class, enumDecl->getLocStart());
        return true;
    }

    bool Cpp14scanner::VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl) {
        if (!inProcessedFile(recordDecl, f_context))
            return true;
        if (recordDecl->isUnion()) {
            //TODO hasUninitializedReferenceMember, isTriviallyCopyable
        }
        return true;
    }

    bool Cpp14scanner::VisitStringLiteral(clang::StringLiteral *literal) {
        if (!inProcessedFile(literal, f_context))
            return true;
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
        if (i.find('\'') != string::npos) {
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
        if (f.find('\'') != string::npos) {
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

    bool Cpp14scanner::VisitSizeOfPackExpr(clang::SizeOfPackExpr *sizeofPackExpr) {
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

}
