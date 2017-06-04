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

#include "tool.h"

#include "default.h"
#include "delete.h"
#include "auto.h"
#include "decltype.h"
#include "lambda.h"
#include "constuctor_delegation.h"
#include "raw_string.h"
#include "implict_sizeof.h"
#include "digit_separators.h"
#include "alias_type.h"
#include "uniform_initialization.h"
#include "explicit_conversion.h"
#include "in_class_init.h"
#include "range_based_for.h"
#include "strongly_typed_enum.h"
#include "user_literals.h"
#include "final.h"
#include "override.h"
#include"binary_literals.h"
#include "list_init.h"
#include "static_assert.h"
#include "trailing_return.h"
#include "func_templ_default_args.h"
#include "nullptr.h"
#include "alignof.h"
#include "alignas.h"
#include "unicode_literals.h"
#include "inline_namespace.h"
#include "unrestricted_unions.h"
#include "variable_template.h"
#include "alias_template.h"
#include "constexpr.h"
#include "noexcept.h"
#include "variadic_template.h"

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    Cpp14RegressASTConsumer::Cpp14RegressASTConsumer(features::type feature,
                                                     clang::CompilerInstance *ci) {
        switch (feature) {
            // @formatter:off
            case features::alias_template : f_visitor = new AliasTemplateSearcher(ci); break;
            case features::alias_type : f_visitor = new AliasTypeReplacer(ci); break;
            case features::alignas_specifier : f_visitor = new AlignasSearcher(ci); break;
            case features::alignof_operator : f_visitor = new AlignofSearcher(ci); break;
            //case features::attributes : break; //TODO
            case features::auto_keyword : f_visitor = new AutoReplacer(ci); break;
            case features::binary_literals : f_visitor = new BinaryLiteralReplacer(ci); break;
            case features::constexpr_keyword : f_visitor = new ConstexprSearcher(ci); break;
            case features::constuctor_delegation : f_visitor = new ConstructorDelegationReplacer(ci); break;
            case features::decltype_keyword : f_visitor = new DecltypeReplacer(ci); break;
            case features::default_keyword : f_visitor = new DefaultReplacer(ci); break;
            case features::delete_keyword : f_visitor = new DeleteReplacer(ci); break;
            case features::digit_separators : f_visitor = new DigitSeparatorReplacer(ci); break;
            case features::explicit_specifier : f_visitor = new ExplicitConversionReplacer(ci); break;
            //case features::extern_template : break; //TODO
            case features::final_specifier : f_visitor = new FinalReplacer(ci); break;
            case features::func_templ_default_args : f_visitor = new FuncTemplDefaultArgsSearcher(ci); break;
            case features::improved_enum : f_visitor = new ImprovedEnumReplacer(ci); break;
            case features::initializer_list : f_visitor = new ListInitSearcher(ci); break;
            case features::inline_namespace : f_visitor = new InlineNamespaceSearcher(ci); break;
            case features::lambda : f_visitor = new LambdaReplacer(ci); break;
            //case features::long_long_int : break; //TODO
            case features::member_init : f_visitor = new MemberInitReplacer(ci); break;
            case features::member_sizeof : f_visitor = new MemberSizeofReplacer(ci); break;
            case features::noexcept_keyword : f_visitor = new NoexceptSearcher(ci); break;
            case features::null_pointer_constant : f_visitor = new NullPtrReplacer(ci); break;
            case features::override_specifier : f_visitor = new OverrideReplacer(ci); break;
            case features::range_based_for : f_visitor = new RangeBasedForReplacer(ci); break;
            case features::raw_string_literals : f_visitor = new RawStringReplacer(ci); break;
            case features::static_assert_decl : f_visitor = new StaticAssertSearcher(ci); break;
            case features::trailing_return : f_visitor = new TrailingReturnSearcher(ci); break;
            case features::unicode_string_literals : f_visitor = new UnicodeStringLiteralSearcher(ci); break;
            case features::uniform_initialization : f_visitor = new UniformInitReplacer(ci); break;
            case features::unrestricted_unions : f_visitor = new UnrestrictedUnionsSearcher(ci); break;
            case features::user_defined_literals : f_visitor = new UserLiteralReplacer(ci); break;
            case features::variable_templates : f_visitor = new VariableTemplateSearcher(ci); break;
            case features::variadic_templates :  f_visitor = new VariadicTemplateSearcher(ci); break; //TODO
            case features::SIZE : break;
            // @formatter:on
        }
    }

}