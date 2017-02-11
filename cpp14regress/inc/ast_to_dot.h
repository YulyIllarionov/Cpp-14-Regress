
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

    struct ast_graph_node {
        std::string name;
        std::string description;

        ast_graph_node(std::string name_, std::string description_) :
                name(name_), description(description_) {}

        ast_graph_node(clang::Stmt* stmt, clang::ASTContext* context);
        ast_graph_node(clang::Decl* decl, clang::ASTContext* context);
        ast_graph_node(clang::Type* type, clang::ASTContext* context);
    };

    typedef std::pair<ast_graph_node*, ast_graph_node*> ast_graph_edge;

    struct ast_graph
    {
    private:
        std::vector<ast_graph_node*> f_nodes;
        std::vector<ast_graph_edge*> f_edges;
    public:
        ast_graph(clang::Stmt* stmt, clang::ASTContext* context);
        bool to_dot_file(std::string file_name);
        ~ast_graph();
    private:
        void recursive_visit(clang::Stmt *stmt, ast_graph_node *agn_f, clang::ASTContext* context);

    };

}