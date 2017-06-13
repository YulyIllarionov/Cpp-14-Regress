
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
#include "ast_to_dot.h"

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

    std::string LambdaHeaderName::toString(unsigned i, string seed) {
        return std::string(replacement::seed + "_lambda_for_" + seed + "_" + std::to_string(i) + ".def");
    }

    std::string LambdaHeaderGuard::toString(unsigned i, string seed) {
        std::string seedUpper(replacement::seed);
        std::transform(seedUpper.begin(), seedUpper.end(), seedUpper.begin(), ::toupper);
        std::string seedCurrentUpper(seed);
        std::transform(seedCurrentUpper.begin(), seedCurrentUpper.end(), seedCurrentUpper.begin(), ::toupper);
        std::transform(seedCurrentUpper.begin(), seedCurrentUpper.end(), seedCurrentUpper.begin(),
                       [](char c) -> char {
                           if (isdigit(c) || isalpha(c))
                               return c;
                           return '_';
                       });
        return std::string(seedUpper + "_LAMBDA_FOR_" + seedCurrentUpper + "_" + std::to_string(i) + "_DEF");
    }

    std::string GenericType::toString(unsigned i) {
        return std::string("type" + std::to_string(i));
    }

    /*bool LambdaReplacer::VisitCallExpr(CallExpr *lambdaCall) {
        if (!fromUserFile(lambdaCall, f_sourceManager))
            return true;
        LambdaSearcher ls;
        ls.TraverseStmt(lambdaCall);
        if (auto lambda = ls.found()) {
            cout << "Labmda found" << endl;
            f_rewriter->InsertTextAfterToken(lambda->getLocEnd(), ".operator()");
        }
        return true;
    }*/

    //TODO check Types
    //TODO capture this
    bool LambdaReplacer::VisitLambdaExpr(LambdaExpr *lambda) {
        if (!fromUserFile(lambda, f_sourceManager))
            return true;

        PrintingPolicy pp(*f_langOptions);

        FileID fid = f_sourceManager->getFileID(lambda->getLocStart());
        string folder = asFolder(f_sourceManager->getFileEntryForID(fid)->getDir()->getName());
        string seedOfFile(f_sourceManager->getFileEntryForID(fid)->getName());
        seedOfFile = removeExtension(pathPopBack(seedOfFile));
        ofstream header(folder + LambdaHeaderNameGenerator::toString(seedOfFile));
        if (!header.is_open()) {
            f_rewriter->InsertTextBefore(lambda->getLocStart(), Comment::block(
                    replacement::info(type(), replacement::result::found)));
            return true;
        }

        header << "#ifndef " << LambdaHeaderGuardGenerator::toString(seedOfFile) << endl
               << "#define " << LambdaHeaderGuardGenerator::toString(seedOfFile) << endl << endl;

        header << "//lambda at:"
               << lambda->getLocStart().printToString(*f_sourceManager) << endl;

        CXXRecordDecl *lambdaClass = lambda->getLambdaClass();
        DenseMap<const VarDecl *, FieldDecl *> clangCaptures;
        FieldDecl *thisField;
        lambdaClass->getCaptureFields(clangCaptures, thisField);

        //If lambda template
        InParentTemplateSearcher ipts(f_astContext);
        if (auto templateDecl = ipts.find(lambda)) {
            SourceLocation templateEnd = findTokenBeginBeforeLoc(templateDecl->getLocation(),
                                                                 tok::TokenKind::greater, 1, f_astContext);
            SourceRange templateRange(templateDecl->getLocStart(), templateEnd);
            string templateStr;
            if (templateRange.isInvalid()) {
                templateStr = Comment::block(replacement::info(type(), replacement::result::error) +
                                             " in template declaration");
            } else {
                templateStr = toString(templateRange, f_astContext);
            }
            header << templateStr << endl;

        }
        cout << "Capture: " << toString(lambda->getIntroducerRange(), f_astContext) << " " << endl;

        //Lambda Class
        header << "class " << LambdaClassNameGenerator::toString() << " {" << endl;
        if (lambda->capture_begin() != lambda->capture_end())
            header << "private:" << endl;

        //Lambda class fields
        for (auto it = lambda->capture_begin(); it != lambda->capture_end(); it++) {
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
               << " operator()(";
        auto tn = typeNames.begin();
        for (size_t i = 0; i != lambdaFunction->param_size();) {
            ParmVarDecl *parameter = lambdaFunction->getParamDecl(i);
            string paramType;
            if (generics) {
                if (find(generics->begin(), generics->end(), parameter) != generics->end()) {
                    paramType = *tn++;
                }
            } else {
                paramType = parameter->getType().getAsString(pp);
            }
            header << paramType << " "
                   << parameter->getQualifiedNameAsString()
                   << ((++i != lambdaFunction->param_size()) ? ", " : "");
        }
        header << ") const ";

        //Lambda class operator() body
        header << toString(lambda->getBody(), f_astContext) << endl;
        header << "};\n" << endl;

        header << "#endif " << Comment::block(LambdaHeaderGuardGenerator::toString(seedOfFile)) << endl;
        header.close();

        replacement::result res = replacement::result::found;
        SourceLocation includeLoc = IncludeLocSearcher(f_astContext).find(lambda);
        if (includeLoc.isValid()) {
            //Include
            string incl("#include \"" + LambdaHeaderNameGenerator::toString(seedOfFile) + "\"\n");
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
            //Check if lambda called after body
            SourceLocation insertLoc = Lexer::getLocForEndOfToken(
                    lambda->getLocEnd(), 0, *f_sourceManager, *f_langOptions);
            Token token;
            do {
                if (Lexer::getRawToken(insertLoc, token, *f_sourceManager, *f_langOptions, true)) {
                    res = replacement::result::found;
                    break;
                }
                insertLoc = token.getEndLoc();
            } while (token.is(tok::TokenKind::comment));
            if (token.is(tok::TokenKind::l_paren)) {
                f_rewriter->InsertTextAfterToken(lambda->getLocEnd(), ".operator()");
            }

            res = replacement::result::replaced;
        }
        f_rewriter->InsertTextBefore(lambda->getLocStart(),
                                     Comment::block(replacement::info(type(), res)));
        LambdaClassNameGenerator::generate();
        LambdaHeaderNameGenerator::generate();
        LambdaHeaderGuardGenerator::generate();
        return true;
    }

    bool IncludeLocSearcher::checkDecl(const Decl *decl) {
        if (isa<NamespaceDecl>(decl) || isa<TranslationUnitDecl>(decl)) { //TODO first not last
            return true;
        }
        f_location = decl->getLocStart();
        return false;
    }

    SourceLocation IncludeLocSearcher::find(const Stmt *stmt) {
        f_found = false;
        visitStmt(stmt);
        return f_location;
    }

    bool InParentTemplateSearcher::checkDecl(const Decl *decl) {
        if (auto templateDecl = dyn_cast_or_null<RedeclarableTemplateDecl>(decl)) {
            f_templateDecl = templateDecl;
            return true;
        }
        return false;
    }

    const RedeclarableTemplateDecl *InParentTemplateSearcher::find(const clang::Stmt *stmt) {
        f_templateDecl = nullptr;
        visitStmt(stmt);
        return f_templateDecl;
    }

}
