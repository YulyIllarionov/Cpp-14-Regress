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

#include "ast_to_dot.h"
#include "utils.h"

#include <string>
#include <utility>
#include <vector>

namespace cpp14regress {

    using namespace std;
    using namespace clang;

    ast_graph_node::ast_graph_node(Stmt *stmt, ASTContext* context) {
        name = stmt->getStmtClassName();
        description = stringFromStmt(stmt, context);
    }

    ast_graph_node::ast_graph_node(Decl *decl, ASTContext* context) {
        name = decl->getDeclKindName();
        description = stringFromDecl(decl, context);
    }

    ast_graph_node::ast_graph_node(Type *type, ASTContext* context) {
        name = type->getTypeClassName();
        description = "";
    }

    void ast_graph::recursive_visit(Stmt *stmt, ast_graph_node *agn_f, ASTContext* context) {
        for (Stmt *subStmt : stmt->children()) {
            if (subStmt != nullptr) {
                bool visit = false;
                ast_graph_node *agn_c;
                ast_graph_edge *age;
                if (Stmt *s = dyn_cast<Stmt>(subStmt)) {
                    agn_c = new ast_graph_node(s, context);
                    visit = true;
                }
                if (Decl *d = dyn_cast<Decl>(subStmt))
                    agn_c = new ast_graph_node(d, context);
                if (Type *t = dyn_cast<Type>(subStmt))
                    agn_c = new ast_graph_node(t, context);

                if (agn_c == nullptr)
                    agn_c = new ast_graph_node("unknown", "");
                age = new ast_graph_edge(agn_f, agn_c);
                f_nodes.push_back(agn_c);
                f_edges.push_back(age);
                if (visit)
                    recursive_visit(subStmt, agn_c, context);
            }
        }
    }

    ast_graph::ast_graph(Stmt *stmt, ASTContext* context)
    {
        if (stmt == nullptr)
            return;

        ast_graph_node* agn = new ast_graph_node(stmt, context);
        f_nodes.push_back(agn);
        recursive_visit(stmt, agn, context);
    }
}
