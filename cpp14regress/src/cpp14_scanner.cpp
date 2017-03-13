
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
#include "utils.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool Cpp14scanner::VisitCXXForRangeStmt(CXXForRangeStmt *for_loop) {
        if (!inProcessedFile(for_loop, f_context))
            return true;
        f_stat.increment(cpp14features::range_based_for);
        return true;
    }

    bool Cpp14scanner::VisitLambdaExpr(clang::LambdaExpr *lambda) {
        if (!inProcessedFile(lambda, f_context))
            return true;
        f_stat.increment(cpp14features::lambda_function);
        return true;
    }

    bool Cpp14scanner::VisitDeclaratorDecl(clang::DeclaratorDecl *valueDecl) {
        if (!inProcessedFile(valueDecl, f_context))
            return true;
        if (isa<AutoType>(valueDecl->getType().getTypePtr())) {
            f_stat.increment(cpp14features::auto_keyword);
            cout << toSting<>(valueDecl, f_context) << endl;
        } else if (isa<DecltypeType>(valueDecl->getType().getTypePtr()))
            f_stat.increment(cpp14features::decltype_keyword);
        return true;
    }

    bool Cpp14scanner::VisitCXXMethodDecl(clang::CXXMethodDecl *method) {
        if (!inProcessedFile(method, f_context))
            return true;
        if (method->isExplicitlyDefaulted())
            f_stat.increment(cpp14features::default_specifier);
        if (method->isDeleted())
            f_stat.increment(cpp14features::delete_specifier);
        return true;
    }

    bool Cpp14scanner::VisitCXXConversionDecl(clang::CXXConversionDecl *conversion_method) {
        if (!inProcessedFile(conversion_method, f_context))
            return true;
        if (conversion_method->isExplicitSpecified())
            f_stat.increment(cpp14features::explicit_specifier);
        return true;
    }

    bool Cpp14scanner::VisitCXXConstructorDecl(clang::CXXConstructorDecl *constructor) {
        if (!inProcessedFile(constructor, f_context))
            return true;
        if (constructor->isExplicitSpecified())
            f_stat.increment(cpp14features::explicit_specifier);
        if (constructor->isDelegatingConstructor())
            f_stat.increment(cpp14features::constuctor_delegation);
        return true;
    }

    bool Cpp14scanner::VisitEnumDecl(clang::EnumDecl *enum_decl) {
        if (!inProcessedFile(enum_decl, f_context))
            return true;
        if (enum_decl->isScopedUsingClassTag())
            f_stat.increment(cpp14features::enum_class);
        return true;
    }

    bool Cpp14scanner::VisitCXXRecordDecl(clang::CXXRecordDecl *record_decl) {
        if (!inProcessedFile(record_decl, f_context))
            return true;
        if (record_decl->isUnion()) {
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
            f_stat.increment(cpp14features::unicode_string_literals);
        return true;
    }

    bool Cpp14scanner::VisitIntegerLiteral(clang::IntegerLiteral *literal) {
        if (!inProcessedFile(literal, f_context))
            return true;
        string i = toSting(literal, f_context);
        if (i.find('\'') != string::npos)
            f_stat.increment(cpp14features::digit_separators);
        return true;
    }

    bool Cpp14scanner::VisitFloatingLiteral(clang::FloatingLiteral *literal) {
        if (!inProcessedFile(literal, f_context))
            return true;
        string f = toSting(literal, f_context);
        if (f.find('\'') != string::npos)
            f_stat.increment(cpp14features::digit_separators);
        return true;
    }

    bool Cpp14scanner::VisitExpr(clang::Expr *expr) {
        if (!inProcessedFile(expr, f_context))
            return true;
        if (expr->isNullPointerConstant(*f_context,
                                        Expr::NullPointerConstantValueDependence::NPC_NeverValueDependent)
            == Expr::NullPointerConstantKind::NPCK_CXX11_nullptr) {
            f_stat.increment(cpp14features::null_pointer_constant);
        }
        return true;
    }

}