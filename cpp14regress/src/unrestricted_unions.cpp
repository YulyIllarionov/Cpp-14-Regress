
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

#include "unrestricted_unions.h"
#include "utils.h"
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    bool UnrestrictedUnionsSearcher::VisitCXXRecordDecl(clang::CXXRecordDecl *recordDecl) {
        if (!fromUserFile(recordDecl, f_sourceManager))
            return true;

        if (recordDecl->isUnion()) { //TODO fix
            bool restricted = false;
            for (auto it = recordDecl->field_begin(); it != recordDecl->field_end(); it++) {
                if (!(*it)->getType().isCXX98PODType(*f_astContext)) {
                    restricted = true;
                    break;
                }
            }
            if (restricted) {
                f_rewriter->InsertTextBefore(recordDecl->getLocStart(), Comment::line(
                        replacement::info(type(), replacement::result::found)) + " \n");
            }
        }
        return true;
    }

}