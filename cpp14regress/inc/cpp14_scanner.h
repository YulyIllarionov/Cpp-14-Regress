#ifndef CPP14REGRESS_SCANNER_H
#define CPP14REGRESS_SCANNER_H

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

#include "base_types.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    enum class cpp14features {
        begin,
        auto_keyword = begin, //found
        decltype_keyword, //found
        constexpr_keyword, //found
        extern_template, //?
        default_specifier, //found
        delete_specifier, //found
        override_specifier, //?
        final_specifier, //?
        explicit_specifier, //found
        initializer_list, //?
        uniform_initialization, //?
        range_based_for, //found
        lambda_function, //found
        alternative_function_syntax, //?
        constuctor_delegation, //found
        null_pointer_constant, //?
        enum_class, //found
        right_angle_bracket, //?
        typedef_template, //?
        unrestricted_unions, //? //boost::variant или placement-new
        variadic_templates, //?
        unicode_string_literals, //found
        user_defined_literals, //?
        long_long_int, //?
        implict_sizeof, //?
        noexcept_keyword, //found
        alignof_operator, //?
        alignas_operator, //?
        attributes, //?
        variable_templates, //?
        digit_separators, //found
        end
    };

    class cpp14features_stat {
    private:
        static unsigned const f_size = (int) cpp14features::end - (int) cpp14features::begin;
        std::vector<clang::SourceLocation> f_features[f_size];

    public:
        cpp14features_stat() {}

        static constexpr unsigned size() { return (f_size); }

        size_t size(cpp14features i) { return f_features[(int) i].size(); }

        std::vector<clang::SourceLocation> &operator[](cpp14features f) { return f_features[(int) f]; }

        void push(cpp14features f, clang::SourceLocation sl) { f_features[(int) f].push_back(sl); }

        static std::string toString(cpp14features f);
    };

    class Cpp14scanner : public clang::RecursiveASTVisitor<Cpp14scanner> {
    private:
        cpp14features_stat f_stat;
        clang::ASTContext *f_context;

    public:

        Cpp14scanner(clang::ASTContext *context) : f_context(context) {}

        cpp14features_stat *getStat() { return &f_stat; }; //TODO bad * ?

        virtual void EndFileAction();

        //range_based_for
        virtual bool VisitCXXForRangeStmt(clang::CXXForRangeStmt *forLoop);

        //lambda_function
        virtual bool VisitLambdaExpr(clang::LambdaExpr *lambdaExpr);

        //auto_keyword
        // decltype_keyword
        virtual bool VisitDeclaratorDecl(clang::DeclaratorDecl *valueDecl);

        //constexpr keyword //TODO by FunctionDecl::isConstexpr()
        virtual bool VisitFunctionDecl(clang::FunctionDecl *functionDecl);

        //default_specifier //TODO by CXXMethodDecl::isExplicitlyDefaulted()
        //delete_specifier //TODO by CXXMethodDecl::isDeleted()
        virtual bool VisitCXXMethodDecl(clang::CXXMethodDecl *methodDecl);

        //explicit_specifier //TODO by CXXConversionDecl::isExplicitSpecified()
        virtual bool VisitCXXConversionDecl(clang::CXXConversionDecl *conversionMethod);

        //constuctor_delegation //TODO by CXXConstructorDecl::isDelegatingConstructor
        //explicit_specifier //TODO by CXXConstructorDecl::isExplicitSpecified()
        virtual bool VisitCXXConstructorDecl(clang::CXXConstructorDecl *constructorDecl);

        //null_pointer_constant TODO by isNullPointerConstant()
        virtual bool VisitExpr(clang::Expr *expr);

        //noexcept_keyword
        virtual bool VisitCXXNoexceptExpr(clang::CXXNoexceptExpr *noexceptExpr);

        //enum_class //TODO by EnumDecl::isScoped() and EnumDecl::isScopedUsingClassTag()
        virtual bool VisitEnumDecl(clang::EnumDecl *enumDecl);

        //unrestricted_unions //TODO
        virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl);

        //unicode_string_literals //TODO
        virtual bool VisitStringLiteral(clang::StringLiteral *literal);

        //digit_separators //TODO
        virtual bool VisitIntegerLiteral(clang::IntegerLiteral *literal);

        //digit_separators //TODO
        virtual bool VisitFloatingLiteral(clang::FloatingLiteral *literal);
    };

}

#endif /*CPP14REGRESS_SCANNER_H*/