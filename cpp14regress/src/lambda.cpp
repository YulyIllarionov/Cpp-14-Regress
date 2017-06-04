
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

#include "lambda.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

namespace cpp14regress {

    using namespace std;
    using namespace clang;
    using namespace llvm;

    /*std::string VariableToField::toString() {
        return string("f_" + f_variable->getNameAsString());
    }*/

    std::string LambdaClassName::toString(unsigned i) {
        return std::string("__" + replacement::seed + "Lambda" + std::to_string(i));
    }

    std::string LambdaHeaderName::toString(unsigned i) {
        return std::string(replacement::seed + "_lambda_" + std::to_string(i) + ".def");
    }

    std::string LambdaHeaderGuard::toString(unsigned i) {
        std::string seedUpper;
        std::transform(replacement::seed.begin(), replacement::seed.end(), seedUpper.begin(),
                       ::toupper);
        return std::string(seedUpper + "_LAMBDA_" + std::to_string(i) + "_H");
    }

    std::string GenericType::toString(unsigned i) {
        return std::string("type" + std::to_string(i));
    }

    //TODO check Types
    //TODO capture this
    bool LambdaReplacer::VisitLambdaExpr(LambdaExpr *lambda) {
        if (!fromUserFile(lambda, f_sourceManager))
            return true;

        PrintingPolicy pp(*f_langOptions);

        FileID fid = f_sourceManager->getFileID(lambda->getLocStart());
        string folder = asFolder(f_sourceManager->getFileEntryForID(fid)->getDir()->getName());
        ofstream header(folder + LambdaHeaderNameGenerator::toString());
        header << "#ifndef " << LambdaHeaderGuardGenerator::toString() << endl
               << "#define " << LambdaHeaderGuardGenerator::toString() << endl << endl;

        header << "//lambda at:"
               << lambda->getLocStart().printToString(*f_sourceManager) << endl;

        CXXRecordDecl *lambdaClass = lambda->getLambdaClass();
        DenseMap<const VarDecl *, FieldDecl *> clangCaptures;
        FieldDecl *thisField;
        lambdaClass->getCaptureFields(clangCaptures, thisField);

        //Lambda Class
        header << "class " << LambdaClassNameGenerator::toString() << " {" << endl;
        if (lambda->capture_begin() != lambda->capture_end())
            header << "private:" << endl;

        //Lambda class fields
        for (auto it = lambda->captures().begin(); it != lambda->captures().end(); it++) {
            VarDecl *captured_var = it->getCapturedVar();
            header << ((lambda->isMutable()) ? "mutable " : "")
                   << clangCaptures[captured_var]->getType().getAsString(pp) << " "
                   << captured_var->getNameAsString() << ";" << endl;
        }

        header << "public:" << endl;
        //Lambda class constructor
        header << LambdaClassNameGenerator::toString() << " (";
        for (auto it = lambda->capture_begin(); it != lambda->capture_end();) {
            VarDecl *captured_var = it->getCapturedVar();
            header << clangCaptures[captured_var]->getType().getAsString(pp) << " "
                   << captured_var->getNameAsString() << "_"
                   << ((++it != lambda->capture_end()) ? ", " : "");
        }
        header << ") ";
        if (lambda->capture_begin() != lambda->capture_end())
            header << ": ";
        //Lambda class constructor body
        for (auto it = lambda->capture_begin(); it != lambda->capture_end();) {
            VarDecl *captured_var = it->getCapturedVar();
            header << captured_var->getNameAsString()
                   << "(" << captured_var->getNameAsString() << "_)"
                   << ((++it != lambda->capture_end()) ? ", " : "");
        }
        header << " {}" << endl;

        //Lambda class operator()
        FunctionDecl *lambdaFunction = lambda->getCallOperator();
        std::vector<string> typeNames;
        auto generics = lambda->getTemplateParameterList();
        if (generics) {
            header << "template <";
            for (size_t i = 0; i < generics->size();) {
                typeNames.push_back(GenericTypeGenerator::toString());
                header << "typename " << typeNames.back()
                       << ((++i != generics->size()) ? ", " : ">");
            }
            header << endl;
        }
        GenericTypeGenerator::reset();

        header << lambdaFunction->getReturnType().getAsString(pp)
               << " operator() (";
        auto tn = typeNames.begin();
        for (size_t i = 0; i != lambdaFunction->param_size();) {
            ParmVarDecl *parameter = lambdaFunction->getParamDecl(i);
            header << ((isa<TemplateTypeParmType>(*parameter->getType())) ? *tn++ :
                       parameter->getType().getAsString(pp)) << " "
                   << parameter->getQualifiedNameAsString()
                   << ((++i != lambdaFunction->param_size()) ? ", "
                                                             : "");
        }
        header << ") const ";
        //Lambda class operator() body
        header << toString(lambda->getBody(), f_astContext) << endl;
        header << "};\n" << endl;

        header << "#endif " << Comment::block(LambdaHeaderGuardGenerator::toString()) << endl;

        replacement::result res = replacement::result::found;
        SourceLocation includeLoc = IncludeLocSearcher(f_astContext).find(lambda);
        if (includeLoc.isValid()) {
            //Include
            string incl("#include \"" + LambdaHeaderNameGenerator::toString() + "\"\n");
            f_rewriter->InsertTextBefore(includeLoc, incl);
            //f_rewriter->InsertTextBefore(includeLoc, Comment::line(
            //        replacement::info(type(), replacement::result::replaced)) + "\n");

            //Lambda class call
            string call(LambdaClassNameGenerator::toString() + "(");
            for (auto it = lambda->capture_init_begin(); it != lambda->capture_init_end();) {
                auto capture = *it;
                call += toString(capture, f_astContext);
                call += ((++it != lambda->capture_init_end()) ? ", " : "");
            }
            call += ")";
            f_rewriter->ReplaceText(lambda->getSourceRange(), call);
            res = replacement::result::replaced;
        }
        f_rewriter->InsertTextBefore(lambda->getLocStart(),
                                     Comment::block(replacement::info(type(), res)));
        LambdaClassNameGenerator::generate();
        LambdaHeaderNameGenerator::generate();
        LambdaHeaderGuardGenerator::generate();
        return true;
    }

    void IncludeLocSearcher::visit(const AnyNode &node) {
        if (auto d = node.get<Decl>())
            visitDecl(d);
        if (auto s = node.get<Stmt>())
            visitStmt(s);
        if (auto t = node.get<Type>())
            visitType(t);
    }

    void IncludeLocSearcher::visitDecl(const Decl *decl) {
        if (!f_found && decl) {
            if (isa<NamespaceDecl>(decl) || isa<TranslationUnitDecl>(decl)) {
                f_found = true;
                return;
            }
            f_location = decl->getLocStart();
            const auto &parents = f_astContext->getParents(*decl);
            for (auto parent : parents) {
                visit(parent);
            }
        }
    }

    void IncludeLocSearcher::visitStmt(const clang::Stmt *stmt) {
        if (!f_found && stmt) {
            f_location = stmt->getLocStart();
            const auto &parents = f_astContext->getParents(*stmt);
            for (auto parent : parents) {
                visit(parent);
            }
        }
    }

    void IncludeLocSearcher::visitType(const clang::Type *type) {
        if (!f_found && type) {
            const auto &parents = f_astContext->getParents(*type);
            for (auto parent : parents) {
                visit(parent);
            }
        }
    }

    SourceLocation IncludeLocSearcher::find(const Stmt *stmt) {
        f_found = false;
        visitStmt(stmt);
        return f_location;
    }
}
