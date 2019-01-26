//
//  Compiler.hpp
//  compiler
//
//  Created by jing hong chen on 1/11/19.
//  Copyright © 2019 jing hong chen. All rights reserved.
//

#ifndef Compiler_h
#define Compiler_h

#include <iostream>
#include <string>
#include "Scanner.hpp"
#include "SymTable.hpp"

class Compiler {
private:
    std::string sourceCodeFilename;
    SymTable symbolTable;
    std::string currentClass;
    std::string currentMethod;
    std::string currentParam;

public:
    Compiler(std::string filename) {
        sourceCodeFilename = filename;
        currentClass = "";
        currentMethod = "";
        currentParam = "";
    }
    
    void syntaxError(Token token, std::string expected) {
        std::cout << token.lineNumber << ": Found " << token.lexeme << " expecting " << expected << std::endl;
        exit(2);
    }
    
    bool isAtype(Token token) {
        return (token.type == T_Identifier
                || token.lexeme == "int"
                || token.lexeme == "char"
                || token.lexeme == "bool"
                || token.lexeme == "void"
                || token.lexeme == "sym");
    }
    
    bool isAexpressionZ(Token token) {
        return (token.lexeme == "="
                || token.lexeme == "&&"
                || token.lexeme == "||"
                || token.lexeme == "=="
                || token.lexeme == "!="
                || token.lexeme == "<="
                || token.lexeme == ">="
                || token.lexeme == "<"
                || token.lexeme == ">"
                || token.lexeme == "+"
                || token.lexeme == "-"
                || token.lexeme == "*"
                || token.lexeme == "/");
    }
    
    bool isNumericLiteral(Token token, Token next) {
        return token.type == T_Number ||
        ((token.lexeme == "+" || token.lexeme == "-") && next.type == T_Number);
    }
    
    void numeric_literal(Scanner& scanner) {
        std::string symValue = "";
        if (scanner.getToken().lexeme == "+" || scanner.getToken().lexeme == "-") {
            symValue += scanner.getToken().lexeme;
            scanner.fetchTokens();
            if (scanner.getToken().type == T_Number) {
                symValue += scanner.getToken().lexeme;
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "numeric_literal");
            }
        }
        else if (scanner.getToken().type == T_Number) {
            symValue += scanner.getToken().lexeme;
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "numeric_literal");
        }
        symbolTable.insert("g", "N", symValue, "ilit", "int", "", "", "public");
    }
    
    void argument_list(Scanner& scanner) {
        expression(scanner);
        while (scanner.getToken().lexeme == ",") {
            scanner.fetchTokens();
            expression(scanner);
        }
    }
    
    void fn_arr_member(Scanner& scanner) {
        if (scanner.getToken().lexeme == "(") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme != ")") {
                argument_list(scanner);
            }
            if (scanner.getToken().lexeme == ")") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
        }
        else if (scanner.getToken().lexeme == "[") {
            scanner.fetchTokens();
            expression(scanner);
            if (scanner.getToken().lexeme == "]") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "]");
            }
        }
        else {
            syntaxError(scanner.getToken(), "fn_arr_member");
        }
    }
    
    void member_refz(Scanner& scanner) {
        if (scanner.getToken().lexeme == ".") {
            scanner.fetchTokens();
            if (scanner.getToken().type == T_Identifier) {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "IDENTIFIER");
            }
            if (scanner.getToken().lexeme == "(" || scanner.getToken().lexeme == "[") {
                fn_arr_member(scanner);
            }
            if (scanner.getToken().lexeme == ".") {
                member_refz(scanner);
            }
        }
        else {
            syntaxError(scanner.getToken(), ".");
        }
    }
    
    void new_declaration(Scanner& scanner) {
        if (scanner.getToken().lexeme == "(") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme != ")") {
                argument_list(scanner);
            }
            if (scanner.getToken().lexeme == ")") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
        }
        else if (scanner.getToken().lexeme == "[") {
            scanner.fetchTokens();
            expression(scanner);
            if (scanner.getToken().lexeme == "]") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "]");
            }
        }
        else {
            syntaxError(scanner.getToken(), "new_declaration");
        }
    }
    
    void assignment_expression(Scanner& scanner) {
        if (scanner.getToken().lexeme == "itoa") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "(") {
                scanner.fetchTokens();
                expression(scanner);
                if (scanner.getToken().lexeme == ")") {
                    scanner.fetchTokens();
                }
                else {
                    syntaxError(scanner.getToken(), ")");
                }
            }
            else {
                syntaxError(scanner.getToken(), "(");
            }
        }
        else if (scanner.getToken().lexeme == "atoi") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "(") {
                scanner.fetchTokens();
                expression(scanner);
                if (scanner.getToken().lexeme == ")") {
                    scanner.fetchTokens();
                }
                else {
                    syntaxError(scanner.getToken(), ")");
                }
            }
            else {
                syntaxError(scanner.getToken(), "(");
            }
        }
        else if (scanner.getToken().lexeme == "new") {
            scanner.fetchTokens();
            if (isAtype(scanner.getToken())) {
                scanner.fetchTokens();
                new_declaration(scanner);
            }
            else {
                syntaxError(scanner.getToken(), "type");
            }
        }
        else {
            expression(scanner);
        }
    }
    
    void expressionZ(Scanner& scanner) {
        if (scanner.getToken().lexeme == "=") {
            scanner.fetchTokens();
            assignment_expression(scanner);
        }
        else if (scanner.getToken().lexeme == "&&") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "||") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "==") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "!=") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "<=") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == ">=") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "<") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == ">") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "+") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "-") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "*") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "/") {
            scanner.fetchTokens();
            expression(scanner);
        }
        else {
            syntaxError(scanner.getToken(), "expressionZ");
        }
    }
    
    void expression(Scanner& scanner) {
        if (scanner.getToken().lexeme == "(") {
            scanner.fetchTokens();
            expression(scanner);
            if (scanner.getToken().lexeme == ")") {
                scanner.fetchTokens();
                if (isAexpressionZ(scanner.getToken())) {
                    expressionZ(scanner);
                }
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
        }
        else if (scanner.getToken().lexeme == "true") {
            scanner.fetchTokens();
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else if (scanner.getToken().lexeme == "false") {
            scanner.fetchTokens();
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else if (scanner.getToken().lexeme == "null") {
            scanner.fetchTokens();
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else if (scanner.getToken().lexeme == "this") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == ".") {
                member_refz(scanner);
            }
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else if (isNumericLiteral(scanner.getToken(),scanner.peekToken())) {
            numeric_literal(scanner);
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else if (scanner.getToken().type == T_Character) {
            // todo clit
            scanner.fetchTokens();
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else if (scanner.getToken().type == T_Identifier) {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "(" || scanner.getToken().lexeme == "[") {
                fn_arr_member(scanner);
            }
            if (scanner.getToken().lexeme == ".") {
                member_refz(scanner);
            }
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else {
            syntaxError(scanner.getToken(), "expression");
        }
    }
    
    void case_label(Scanner & scanner) {
        if (scanner.getToken().lexeme == "case") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "case");
        }
        if (scanner.getToken().type == T_Character) {
            // todo literal
            scanner.fetchTokens();
        }
        else if (isNumericLiteral(scanner.getToken(),scanner.peekToken())) {
            numeric_literal(scanner);
        }
        else {
            syntaxError(scanner.getToken(), "literal");
        }
        if (scanner.getToken().lexeme == ":") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), ":");
        }
        statement(scanner);
    }
    
    void case_block(Scanner & scanner) {
        if (scanner.getToken().lexeme == "{") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "{");
        }
        while (scanner.getToken().lexeme == "case") {
            case_label(scanner);
        }
        if (scanner.getToken().lexeme == "}") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "}");
        }
    }
    
    void statement(Scanner & scanner) {
        if (scanner.getToken().lexeme == "{") {
            scanner.fetchTokens();
            while (scanner.getToken().lexeme != "}") {
                statement(scanner);
            }
            scanner.fetchTokens(); // comsume the "}"
        }
        else if (scanner.getToken().lexeme == "if") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "(") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "(");
            }
            expression(scanner);
            if (scanner.getToken().lexeme == ")") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
            statement(scanner);
            if (scanner.getToken().lexeme == "else") {
                scanner.fetchTokens();
                statement(scanner);
            }
        }
        else if (scanner.getToken().lexeme == "while") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "(") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "(");
            }
            expression(scanner);
            if (scanner.getToken().lexeme == ")") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
            statement(scanner);
        }
        else if (scanner.getToken().lexeme == "return") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme != ";") {
                expression(scanner);
            }
            if (scanner.getToken().lexeme == ";") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ";");
            }
        }
        else if (scanner.getToken().lexeme == "cout") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "<<") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "<<");
            }
            expression(scanner);
            if (scanner.getToken().lexeme == ";") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ";");
            }
        }
        else if (scanner.getToken().lexeme == "cin") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == ">>") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ">>");
            }
            expression(scanner);
            if (scanner.getToken().lexeme == ";") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ";");
            }
        }
        else if (scanner.getToken().lexeme == "switch") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "(") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "(");
            }
            expression(scanner);
            if (scanner.getToken().lexeme == ")") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
            case_block(scanner);
        }
        else if (scanner.getToken().lexeme == "break") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == ";") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ";");
            }
        }
        else {
            expression(scanner);
            if (scanner.getToken().lexeme == ";") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ";");
            }
        }
    }
    
    void parameter(Scanner & scanner) {
        if (isAtype(scanner.getToken())) {
            // get param type
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "type");
        }
        if (scanner.getToken().type == T_Identifier) {
            // get param name
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "INDENTIFIER");
        }
        if (scanner.getToken().lexeme == "[") {
            // change param type to array
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "]") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "]");
            }
        }
        // todo param
        // update paramString
    }
    
    void parameter_list(Scanner & scanner) {
        parameter(scanner);
        while (scanner.getToken().lexeme == ",") {
            // update paramString
            scanner.fetchTokens();
            parameter(scanner);
        }
    }
    
    void field_declaration(Scanner & scanner) {
        if (scanner.getToken().lexeme == "(") {
            // todo method
            // record method id
            // enter method
            //std::string paramString = "";
            scanner.fetchTokens();
            if (isAtype(scanner.getToken())) {
                parameter_list(scanner);
            }
            if (scanner.getToken().lexeme == ")") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
            // update paramstring to method
            method_body(scanner);
            // exit method
        }
        else if (scanner.getToken().lexeme == "["
                 || scanner.getToken().lexeme == "="
                 || scanner.getToken().lexeme == ";") {
            // todo instance variable
            if (scanner.getToken().lexeme == "[") {
                scanner.fetchTokens();
                // ???? missing array length ????
                if (scanner.getToken().lexeme == "]") {
                    scanner.fetchTokens();
                }
                else {
                    syntaxError(scanner.getToken(), "]");
                }
            }
            if (scanner.getToken().lexeme == "=") {
                scanner.fetchTokens();
                assignment_expression(scanner);
            }
            if (scanner.getToken().lexeme == ";") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ";");
            }
        }
        else {
            syntaxError(scanner.getToken(), "field_delaration");
        }
    }
    
    void constructor_declaration(Scanner & scanner) {
        if (scanner.getToken().type == T_Identifier) {
            // check class name
            // todo constructor
            // enter constructor
            // paramString = "";
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "(") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "(");
            }
            if (isAtype(scanner.getToken())) {
                parameter_list(scanner);
            }
            if (scanner.getToken().lexeme == ")") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
            // update paramString to constructor
            method_body(scanner);
            // exit constructor
        }
        else {
            syntaxError(scanner.getToken(), "constructor_declaration");
        }
    }
    
    void class_member_declaration(Scanner & scanner) {
        if (scanner.getToken().lexeme == "public" || scanner.getToken().lexeme == "private") {
            //todo get modifier
            scanner.fetchTokens();
            if (isAtype(scanner.getToken())) {
                //todo get type
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "type");
            }
            if (scanner.getToken().type == T_Identifier) {
                //todo get variable or method name
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "IDENTIFIER");
            }
            field_declaration(scanner);
        }
        else {
            constructor_declaration(scanner);
        }
    }
    
    void class_declaration(Scanner & scanner) {
        if (scanner.getToken().lexeme == "class") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "class");
        }
        if (scanner.getToken().type == T_Identifier) {
            //todo class name
            // enter class
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "class_name");
        }
        if (scanner.getToken().lexeme == "{") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "{");
        }
        while (scanner.getToken().lexeme != "}") {
            class_member_declaration(scanner);
        }
        scanner.fetchTokens();  //comsume the closing "}"
        //todo exit class
    }
    
    void variable_declaration(Scanner & scanner) {
        if (isAtype(scanner.getToken())) {
            // get variable kind
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "type");
        }
        if (scanner.getToken().type == T_Identifier) {
            // get variable name
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "INDENTIFIER");
        }
        if (scanner.getToken().lexeme == "[") {
            // change the kind to array
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "]") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "]");
            }
        }
        // todo lvar
        if (scanner.getToken().lexeme == "=") {
            scanner.fetchTokens();
            assignment_expression(scanner);
        }
        if (scanner.getToken().lexeme == ";") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), ";");
        }
    }
    
    void method_body(Scanner & scanner) {
        if (scanner.getToken().lexeme == "{") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "{");
        }
        while (isAtype(scanner.getToken()) && scanner.peekToken().type == T_Identifier) {
            variable_declaration(scanner);
        }
        while (scanner.getToken().lexeme != "}") {
            statement(scanner);
        }
        scanner.fetchTokens();  //consume the closing "}"
    }

    void compiliation_unit(Scanner & scanner) {
        while (scanner.getToken().lexeme == "class") {
            class_declaration(scanner);
        }
        if (scanner.getToken().lexeme == "void") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "void");
        }
        if (scanner.getToken().lexeme == "kxi2019") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "kxi2019");
        }
        if (scanner.getToken().lexeme == "main") {
            //todo main
            // enter main
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "main");
        }
        if (scanner.getToken().lexeme == "(") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "(");
        }
        if (scanner.getToken().lexeme == ")") {
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), ")");
        }
        method_body(scanner);
        // todo exit main
    }
    
    void lexicalAnalysis() {
        Scanner scanner(sourceCodeFilename);
        scanner.fetchTokens();
        while (scanner.peekToken().type != T_EOF) {
            scanner.fetchTokens();
            scanner.printCurrentToken();
        }
        scanner.printNextToken();
    }
    
    void syntaxAnalysis() {
        Scanner scanner(sourceCodeFilename);
        scanner.fetchTokens();  // fetch a token to nextToken
        scanner.fetchTokens();  // fetch a token to currentToken and nextToken
        compiliation_unit(scanner);
        symbolTable.printAll();
    }
    
    void run() {
//        lexicalAnalysis();
        syntaxAnalysis();
    }
};

#endif /* Compiler_h */
