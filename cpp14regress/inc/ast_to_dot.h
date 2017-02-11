
#ifndef CPP14REGRESS_AST_TO_DOT_H
#define CPP14REGRESS_ASST_TO_DOT_H

#endif /*CPP14REGRESS_IDE_AST_TO_DOT_H*/

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

#include "utils.h"

#include <string>
#include <utility>
#include <vector>

namespace cpp14regress {

    typedef clang::Stmt ast_graph_node;
    typedef std::pair<ast_graph_node*, ast_graph_node*> ast_graph_edge;
    typedef std::vector<std::pair<std::string, std::string>> node_inf;

    class ast_graph
    {
    private:
        std::vector<ast_graph_node*> f_nodes;
        std::vector<ast_graph_edge*> f_edges;
        clang::ASTContext* f_context;
    public:
        ast_graph(clang::Stmt* stmt_, clang::ASTContext* context_);
        bool to_dot_file(std::string file_name);
        ~ast_graph(){}
    private:
        void recursive_visit(ast_graph_node *agn_f);
        node_inf get_stmt_inf(clang::Stmt* stmt_);
    };

}