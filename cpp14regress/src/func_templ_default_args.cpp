
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

#include "func_templ_default_args.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;


    bool FuncTemplDefaultArgsSearcher::VisitFunctionTemplateDecl(FunctionTemplateDecl *functionTempl) {
        if (!fromUserFile(functionTempl, f_sourceManager))
            return true;

        auto args = functionTempl->getTemplateParameters();
        bool defaultArg = false;
        for (auto it = args->begin(); it != args->end(); it++) {
            if (auto param = dyn_cast_or_null<TemplateTypeParmDecl>(*it)) {
                if (param->hasDefaultArgument())
                    defaultArg = true;
            } else if (auto param = dyn_cast_or_null<NonTypeTemplateParmDecl>(*it)) {
                if (param->hasDefaultArgument())
                    defaultArg = true;
            }
            if (defaultArg) {
                f_rewriter->InsertTextBefore(functionTempl->getLocStart(), Comment::line(
                        replacement::info(type(), replacement::result::found)) + "\n");
                return true;
            }
        }
        return true;
    }
}