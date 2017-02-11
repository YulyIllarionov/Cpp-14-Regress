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
    void ast_graph::recursive_visit(ast_graph_node *agn_f) {
        for (ast_graph_node *agn_c : agn_f->children()) {
            if (agn_c != nullptr) {
                ast_graph_edge *age = new ast_graph_edge(agn_f, agn_c);
                if(find(f_nodes.begin(), f_nodes.end(), agn_c) == f_nodes.end())
                    f_nodes.push_back(agn_c);
                f_edges.push_back(age);
                recursive_visit(agn_c);
            }
        }
    }

    ast_graph::ast_graph(Stmt *stmt_, ASTContext* context_)
    {
        if (stmt_ == nullptr)
            return;
        f_context = context_;
        f_nodes.push_back(stmt_);
        recursive_visit(stmt_);
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
            ast_graph_node *node = f_nodes[i];
            dot << "n" << to_string(i) << " [label = "
                << node_inf_to_label(get_stmt_inf(node))
                << ", shape = \"box\"" << " ]" << endl;
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

    string ast_graph::string_to_label(string s)
    {
        string::size_type index = 0;
        while (( index = s.find("\"", index)) != string::npos )
        {
            s.replace(index, 1, "\\\"");
            index += 2;
        }
        index = 0;
        while (( index = s.find("\n", index)) != string::npos )
        {
            s.replace(index, 1, "\\l");
            index += 2;
        }
        return s;
    }

    node_inf ast_graph::get_stmt_inf(clang::Stmt* stmt)
    {
        node_inf inf;
        inf.push_back(node_inf_record(string(), stmt->getStmtClassName()));
        inf.push_back(node_inf_record("Source code", string_to_label(stringFromStmt(stmt, f_context))));

        return inf;
    }

    string ast_graph::node_inf_to_label(node_inf inf)
    {
        string table = "<<TABLE>";
        for (node_inf_record record : inf)
        {
            table += "<TR><TD";
            if (record.second.empty())
                table += string(" colspan=\"2\" >" + record.first + "</TD>");
            else
                table += string("> " + record.first + " </TD>" +
                                "<TD> <pre>" + record.second + "</pre> </TD>");
            table += "</TR>";
        }
        table += "</TABLE>>";
        return table;
    }
}

