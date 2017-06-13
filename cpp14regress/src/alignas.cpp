
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

#include "alignas.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;


    bool AlignasSearcher::VisitAttr(clang::Attr *attr) {
        if (!fromUserFile(attr, f_sourceManager))
            return true;
        if (attr->getKind() == attr::Kind::Aligned) { //TODO process only C++11 alignas
            if (!attr->isImplicit()) {
                f_rewriter->InsertTextBefore(attr->getRange().getBegin(), Comment::block(
                        replacement::info(type(),replacement::result::found)));
            }
        }
        return true;
    }
}

