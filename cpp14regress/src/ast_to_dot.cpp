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
#include <fstream>
#include <algorithm>

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
                ast_graph_node *agn_c = new ast_graph_node(subStmt, context);
                ast_graph_edge *age = new ast_graph_edge(agn_f, agn_c);
                f_nodes.push_back(agn_c);
                f_edges.push_back(age);
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

    ast_graph::~ast_graph()
    {
        for (ast_graph_node *n : f_nodes)
            delete n;
    }

    bool ast_graph::to_dot_file(std::string file_name) {
        ofstream dot(file_name);
        if(!dot.is_open())
            return false;

        dot << "digraph graphname {" << endl;
        dot << "rankdir = TB" << endl;
        dot << endl;
        for (size_t i = 0; i < f_nodes.size(); i++)
        {
            const ast_graph_node *n = f_nodes[i];
            string::size_type index = 0;
            string tmp(n->description);
            while (true) {
                index = tmp.find("\"", index);
                if (index == string::npos)
                    break;
                tmp.insert(index, "\\");
                index += 2;
            }
            index = 0;
            while (true) {
                index = tmp.find("\n", index);
                if (index == string::npos)
                    break;
                tmp.replace(index, 1, "\\l");
                index += 1;
            }
            dot << "n" << std::to_string(i) << " [label = \""
                << n->name << "\\n" << "\\n" << tmp << "\\l" << "\" "
                << "shape=\"box\""
                << " ]" << endl;
        }
        dot << endl;
        for (ast_graph_edge *e : f_edges)
        {
            dot << "n" << find(f_nodes.begin(), f_nodes.end(), e->first) - f_nodes.begin() << " -> "
                << "n" << find(f_nodes.begin(), f_nodes.end(), e->second) - f_nodes.begin()  << endl;
        }
        dot << "}" << endl;

        dot.close();
        return true;
    }
}
