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
#include <stack>
#include <map>
#include "Scanner.hpp"
#include "SymTable.hpp"

struct OpRec {
    std::string value;
    int priority;
};

struct SAR {
    int symID;
    int lineNumber;
    std::string type;
    std::string value;
    std::string action;
};

class Compiler {
private:
    std::string sourceCodeFilename;
    SymTable symbolTable;
    std::string currentClass;
    std::string currentMethod;
    std::string currentParam;
    bool flagOfPass;  // false for Pass 1, true for Pass 2
    std::stack<OpRec> OpStack;
    std::stack<SAR> SAS;
    std::map<std::string, int> operatorTable;

public:
    Compiler(std::string filename) {
        sourceCodeFilename = filename;
        currentClass = "";
        currentMethod = "";
        currentParam = "";
        flagOfPass = false;
        
        operatorTable.insert(std::pair<std::string, int>("*",8));
        operatorTable.insert(std::pair<std::string, int>("/",8));
        operatorTable.insert(std::pair<std::string, int>("+",7));
        operatorTable.insert(std::pair<std::string, int>("-",7));
        operatorTable.insert(std::pair<std::string, int>("<",6));
        operatorTable.insert(std::pair<std::string, int>("<=",6));
        operatorTable.insert(std::pair<std::string, int>(">",6));
        operatorTable.insert(std::pair<std::string, int>(">=",6));
        operatorTable.insert(std::pair<std::string, int>("==",5));
        operatorTable.insert(std::pair<std::string, int>("!=",5));
        operatorTable.insert(std::pair<std::string, int>("&&",4));
        operatorTable.insert(std::pair<std::string, int>("||",3));
        operatorTable.insert(std::pair<std::string, int>("=",2));
        operatorTable.insert(std::pair<std::string, int>("(",1));
        operatorTable.insert(std::pair<std::string, int>(")",1));
        operatorTable.insert(std::pair<std::string, int>("[",1));
        operatorTable.insert(std::pair<std::string, int>("]",1));
    }
    
    void syntaxError(Token token, std::string expected) {
        std::cout << token.lineNumber << ": Found " << token.lexeme << " expecting " << expected << std::endl;
        exit(2);
    }
    
    void semanticError(int line,std::string errorMsg) {
        std::cout << line << ": " << errorMsg << std::endl;
        exit(2);
    }
    
    void unexpectedError(std::string errorType) {
        std::cout << errorType << std::endl;
        exit(4);
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
        if (!flagOfPass && symbolTable.searchValue("g", symValue) == 0) {
            symbolTable.insert("g", "N", symValue, "ilit", "int", "", "", "public");
        }
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
                if (flagOfPass) sa_iPush(scanner.getToken());
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "IDENTIFIER");
            }
            if (scanner.getToken().lexeme == "(" || scanner.getToken().lexeme == "[") {
                fn_arr_member(scanner);
            }
            if (flagOfPass) sa_rExist();
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
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            assignment_expression(scanner);
        }
        else if (scanner.getToken().lexeme == "&&") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "||") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "==") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "!=") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "<=") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == ">=") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "<") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == ">") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "+") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "-") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "*") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
        }
        else if (scanner.getToken().lexeme == "/") {
            if (flagOfPass) sa_oPush(scanner.getToken());
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
            if (!flagOfPass && symbolTable.searchValue("g", scanner.getToken().lexeme) == 0) {
                symbolTable.insert("g", "H", scanner.getToken().lexeme, "clit", "char", "", "", "public");
            }
            scanner.fetchTokens();
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else if (scanner.getToken().type == T_Identifier) {
            if (flagOfPass) sa_iPush(scanner.getToken());
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "(" || scanner.getToken().lexeme == "[") {
                fn_arr_member(scanner);
            }
            if (flagOfPass) sa_iExist();
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
            if (!flagOfPass && symbolTable.searchValue("g", scanner.getToken().lexeme) == 0) {
                symbolTable.insert("g", "H", scanner.getToken().lexeme, "clit", "char", "", "", "public");
            }
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
            scanner.fetchTokens(); // consume the closing "}"
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
                if (flagOfPass) sa_EOE();
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ";");
            }
        }
    }
    
    void parameter(Scanner & scanner) {
        std::string paramType;
        std::string paramName;
        int paramLine = 0;
        if (isAtype(scanner.getToken())) {
            paramType = scanner.getToken().lexeme;
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "type");
        }
        if (scanner.getToken().type == T_Identifier) {
            paramName = scanner.getToken().lexeme;
            paramLine = scanner.getToken().lineNumber;
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "INDENTIFIER");
        }
        if (scanner.getToken().lexeme == "[") {
            paramType = "@:" + paramType;
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "]") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "]");
            }
        }
        if (!flagOfPass) {
            if (symbolTable.searchValue("g" + currentClass + currentMethod, paramName) != 0) {
                semanticError(paramLine, "Duplicate variable " + paramName);
            }
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "P", paramName, "param", paramType, "", "", "private");
            currentParam += ("P" + std::to_string(tempId));
        }
    }
    
    void parameter_list(Scanner & scanner) {
        parameter(scanner);
        while (scanner.getToken().lexeme == ",") {
            if (!flagOfPass) {
                currentParam += ",";
            }
            scanner.fetchTokens();
            parameter(scanner);
        }
    }
    
    void field_declaration(Scanner & scanner, std::string modeStr, std::string typeStr, std::string nameStr) {
        if (scanner.getToken().lexeme == "(") {
            int tempId = 0;
            if (!flagOfPass) {
                tempId = symbolTable.insert("g" + currentClass, "M", nameStr, "method", "", typeStr, "", modeStr);
            }
            currentMethod = "." + nameStr;
            currentParam = "";
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
            if (!flagOfPass) {
                symbolTable.updateParam(tempId, "[" + currentParam + "]");
            }
            method_body(scanner);
            currentMethod = "";
        }
        else if (scanner.getToken().lexeme == "["
                 || scanner.getToken().lexeme == "="
                 || scanner.getToken().lexeme == ";") {
            std::string tempType = typeStr;
            if (scanner.getToken().lexeme == "[") {
                tempType = "@:" + tempType;
                scanner.fetchTokens();
               if (scanner.getToken().lexeme == "]") {
                    scanner.fetchTokens();
                }
                else {
                    syntaxError(scanner.getToken(), "]");
                }
            }
            if (!flagOfPass) {
                symbolTable.insert("g" + currentClass, "V", nameStr, "ivar", tempType, "", "", modeStr);
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
            std::string nameStr = scanner.getToken().lexeme;
            int tempId = 0;
            if (!flagOfPass) {
                if (symbolTable.searchValue("g" + currentClass, nameStr) != 0) {
                    if (scanner.peekToken().lexeme == "(") {
                        semanticError(scanner.getToken().lineNumber, "Duplicate function " + nameStr);
                    }
                    else {
                        semanticError(scanner.getToken().lineNumber, "Duplicate variable " + nameStr);
                    }
                }
                tempId = symbolTable.insert("g" + currentClass, "X", nameStr, "Constructor", "", nameStr, "", "public");
            }
            currentMethod = "." + nameStr;
            currentParam = "";
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
            if (!flagOfPass) {
                symbolTable.updateParam(tempId, "[" + currentParam + "]");
            }
            method_body(scanner);
            currentMethod = "";
        }
        else {
            syntaxError(scanner.getToken(), "constructor_declaration");
        }
    }
    
    void class_member_declaration(Scanner & scanner) {
        std::string modeStr;
        std::string typeStr;
        std::string nameStr;
        if (scanner.getToken().lexeme == "public" || scanner.getToken().lexeme == "private") {
            modeStr = scanner.getToken().lexeme;
            scanner.fetchTokens();
            if (isAtype(scanner.getToken())) {
                typeStr = scanner.getToken().lexeme;
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "type");
            }
            if (scanner.getToken().type == T_Identifier) {
                nameStr = scanner.getToken().lexeme;
                if (!flagOfPass && symbolTable.searchValue("g" + currentClass, nameStr) != 0) {
                    if (scanner.peekToken().lexeme == "(") {
                        semanticError(scanner.getToken().lineNumber, "Duplicate function " + nameStr);
                    }
                    else {
                        semanticError(scanner.getToken().lineNumber, "Duplicate variable " + nameStr);
                    }
                }
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "IDENTIFIER");
            }
            field_declaration(scanner, modeStr, typeStr, nameStr);
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
            if (!flagOfPass) {
                if (symbolTable.searchValue("g", scanner.getToken().lexeme) != 0) {
                    semanticError(scanner.getToken().lineNumber, "Duplicate class " + scanner.getToken().lexeme);
                }
                else {
                    symbolTable.insert("g", "C", scanner.getToken().lexeme, "Class", "", "", "", "");
                }
            }
            currentClass = "." + scanner.getToken().lexeme;
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
        currentClass = "";
    }
    
    void variable_declaration(Scanner & scanner) {
        std::string typeStr;
        std::string nameStr;
        if (isAtype(scanner.getToken())) {
            typeStr = scanner.getToken().lexeme;
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "type");
        }
        if (scanner.getToken().type == T_Identifier) {
            nameStr = scanner.getToken().lexeme;
            if (!flagOfPass && symbolTable.searchValue("g" + currentClass + currentMethod, nameStr) != 0) {
                semanticError(scanner.getToken().lineNumber, "Duplicate variable " + nameStr);
            }
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "INDENTIFIER");
        }
        if (scanner.getToken().lexeme == "[") {
            typeStr = "@:" + typeStr;
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "]") {
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "]");
            }
        }
        if (!flagOfPass) {
            symbolTable.insert("g" + currentClass + currentMethod, "L", nameStr, "lvar", typeStr, "", "", "private");
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
            if (!flagOfPass) {
                symbolTable.insert("g", "F", "main", "main", "", "void", "[]", "public");
            }
            currentClass = ".main";
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
        currentClass = "";
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
        if (scanner.getToken().lexeme != "EOF") {
            syntaxError(scanner.getToken(), "EOF");
        }
        else {
            flagOfPass = true;
        }
//        symbolTable.printAll();
    }
    
    void sa_iPush(Token token) {
        SAR newSAR = {0, token.lineNumber, "variable", token.lexeme, "sa_iPush"};
        SAS.push(newSAR);
    }
    
    void sa_lPush() { }
    
    void sa_oPush(Token token) {
        if (token.lexeme == "=" && !OpStack.empty())
            semanticError(token.lineNumber, "Assignment operation error");
        else if (token.lexeme == "(") {
            OpRec newOpRec = {token.lexeme, operatorTable[token.lexeme]};
            OpStack.push(newOpRec);
        }
        else {
            while (true) {
                if (OpStack.empty()) {
                    OpRec newOpRec = {token.lexeme, operatorTable[token.lexeme]};
                    OpStack.push(newOpRec);
                    break;
                }
                else if (OpStack.top().priority < operatorTable[token.lexeme]) {
                    OpRec newOpRec = {token.lexeme, operatorTable[token.lexeme]};
                    OpStack.push(newOpRec);
                    break;
                }
                else {
                    if (OpStack.top().value == "&&") {
                        sa_AndOperator();
                    }
                    else if (OpStack.top().value == "||") {
                        sa_OrOperator();
                    }
                    else if (OpStack.top().value == "==") {
                        sa_EqualOperator();
                    }
                    else if (OpStack.top().value == "!=") {
                        sa_NotEqualOperator();
                    }
                    else if (OpStack.top().value == "<=") {
                        sa_LessEqualOperator();
                    }
                    else if (OpStack.top().value == ">=") {
                        sa_GreaterEqualOperator();
                    }
                    else if (OpStack.top().value == "<") {
                        sa_LessThanOperator();
                    }
                    else if (OpStack.top().value == ">") {
                        sa_GreaterThanOperator();
                    }
                    else if (OpStack.top().value == "+") {
                        sa_AddOperator();
                    }
                    else if (OpStack.top().value == "-") {
                        sa_SubtractOperator();
                    }
                    else if (OpStack.top().value == "*") {
                        sa_MultiplyOperator();
                    }
                    else if (OpStack.top().value == "/") {
                        sa_DivideOperator();
                    }
                    else {
                        semanticError(0, "Unexpected Error on sa_oPush");
                    }
                }
            }
        }
    }
    
    void sa_tPush() { }
    
    void sa_iExist() {
        if (SAS.empty()) unexpectedError("SAS is empty -- #iExist");
        if (SAS.top().type == "variable") {
            int tempId = 0;
            std::string currentPath = "g" + currentClass + currentMethod;
            while (currentPath != "g") {
                tempId = symbolTable.searchValue(currentPath, SAS.top().value);
                if (tempId != 0) break;
                currentPath = currentPath.substr(0, currentPath.find_last_of('.'));
            }
            if (tempId != 0) {
                SAR newSAR = {tempId, SAS.top().lineNumber, "id_sar", SAS.top().value, "sa_iExist"};
                SAS.pop();
                SAS.push(newSAR);
            }
            else {
                semanticError(SAS.top().lineNumber, "Variable \"" + SAS.top().value + "\" not defined");
            }
        }
    }
    
    void sa_vPush() { }
    
    void sa_rExist() {
        if (SAS.empty()) unexpectedError("SAS is empty -- #rExist");
        SAR topSAR = SAS.top();
        SAS.pop();
        if (SAS.empty()) unexpectedError("SAS is empty -- #rExist");
        SAR nextSAR = SAS.top();
        SAS.pop();
        
        if (topSAR.type == "variable") {
            int classID = symbolTable.getClassIDFromObject(nextSAR.symID);
            if (classID == 0) {
                semanticError(topSAR.lineNumber, "Variable \""  + topSAR.value + "\" not defined/public in class \"" + nextSAR.value + "\"");
            }
            int tempId = symbolTable.searchValue("g." + symbolTable.getValue(classID), topSAR.value);
            if (tempId == 0 || symbolTable.getKind(tempId) != "ivar" || symbolTable.getAccessMod(tempId) != "public") {
                semanticError(topSAR.lineNumber, "Variable \""  + topSAR.value + "\" not defined/public in class \"" + nextSAR.value + "\"");
            }
            else {
                SAR newSAR = {tempId, topSAR.lineNumber, "ref_sar", nextSAR.value + "." + topSAR.value, "sa_rExist"};
                SAS.push(newSAR);
            }
        }
    }
    
    void sa_tExist() { }
    
    void sa_BAL() { }
    
    void sa_EAL() { }
    
    void sa_func() { }
    
    void sa_arr() { }
    
    void sa_if() { }
    
    void sa_while() { }
    
    void sa_return() { }
    
    void sa_cout() { }
    
    void sa_cin() { }
    
    void sa_atoi() { }
    
    void sa_itoa() { }
    
    void sa_newObj() { }
    
    void sa_newArray() { }
    
    void sa_CD() { }
    
    void sa_dup() { }
    
    void sa_spawn() { }
    
    void sa_lock() { }
    
    void sa_release() { }
    
    void sa_switch() { }
    
    void sa_DotOperator() { }
    
    void sa_ClosingParenthesis() { }
    
    void sa_ClosingBracket() { }
    
    void sa_Argument() { }
    
    void sa_EOE() {
        while (!OpStack.empty()) {
            if (OpStack.top().value == "=") {
                sa_AssignmetOperator();
            }
            else if (OpStack.top().value == "&&") {
                sa_AndOperator();
            }
            else if (OpStack.top().value == "||") {
                sa_OrOperator();
            }
            else if (OpStack.top().value == "==") {
                sa_EqualOperator();
            }
            else if (OpStack.top().value == "!=") {
                sa_NotEqualOperator();
            }
            else if (OpStack.top().value == "<=") {
                sa_LessEqualOperator();
            }
            else if (OpStack.top().value == ">=") {
                sa_GreaterEqualOperator();
            }
            else if (OpStack.top().value == "<") {
                sa_LessThanOperator();
            }
            else if (OpStack.top().value == ">") {
                sa_GreaterThanOperator();
            }
            else if (OpStack.top().value == "+") {
                sa_AddOperator();
            }
            else if (OpStack.top().value == "-") {
                sa_SubtractOperator();
            }
            else if (OpStack.top().value == "*") {
                sa_MultiplyOperator();
            }
            else if (OpStack.top().value == "/") {
                sa_DivideOperator();
            }
            else {
                semanticError(0, "Unexpected Error on sa_EOE");
            }
        }
        while (!SAS.empty())
            SAS.pop();
    }
    
    void sa_AddOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_AddOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_AddOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if (symbolTable.getType(exp1.symID) == "int" && symbolTable.getType(exp2.symID) == "int") {
            OpStack.pop();
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "int", "", "", "private");
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Add"};
            SAS.push(newSAR);
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " + "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }
    }
    
    void sa_SubtractOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_SubtractOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_SubtractOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if (symbolTable.getType(exp1.symID) == "int" && symbolTable.getType(exp2.symID) == "int") {
            OpStack.pop();
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "int", "", "", "private");
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Subtract"};
            SAS.push(newSAR);
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " - "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }
    }
    
    void sa_MultiplyOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_MultiplyOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_MultiplyOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if (symbolTable.getType(exp1.symID) == "int" && symbolTable.getType(exp2.symID) == "int") {
            OpStack.pop();
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "int", "", "", "private");
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Multiply"};
            SAS.push(newSAR);
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " * "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }
    }
    
    void sa_DivideOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_DivideOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_DivideOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if (symbolTable.getType(exp1.symID) == "int" && symbolTable.getType(exp2.symID) == "int") {
            OpStack.pop();
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "int", "", "", "private");
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Divide"};
            SAS.push(newSAR);
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " / "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }
    }
    
    void sa_AssignmetOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_AssignmetOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_AssignmetOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if (isLValue(exp1.symID) && symbolTable.getType(exp1.symID) == symbolTable.getType(exp2.symID)) {
            OpStack.pop();
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " = "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }
    }
    
    void sa_LessThanOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_LessThanOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_LessThanOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if ((symbolTable.getType(exp1.symID) == "int" || symbolTable.getType(exp1.symID) == "char")
            && symbolTable.getType(exp2.symID) == symbolTable.getType(exp1.symID)) {
            OpStack.pop();
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private");
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Less"};
            SAS.push(newSAR);
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " < "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }
    }
    
    void sa_GreaterThanOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_GreaterThanOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_GreaterThanOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if ((symbolTable.getType(exp1.symID) == "int" || symbolTable.getType(exp1.symID) == "char")
            && symbolTable.getType(exp2.symID) == symbolTable.getType(exp1.symID)) {
            OpStack.pop();
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private");
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Greater"};
            SAS.push(newSAR);
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " > "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }
    }
    
    void sa_EqualOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_EqualOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_EqualOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if (symbolTable.getType(exp2.symID) == symbolTable.getType(exp1.symID)) {
            OpStack.pop();
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private");
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Equal"};
            SAS.push(newSAR);
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " == "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }

    }
    
    void sa_LessEqualOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_LessEqualOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_LessEqualOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if ((symbolTable.getType(exp1.symID) == "int" || symbolTable.getType(exp1.symID) == "char")
            && symbolTable.getType(exp2.symID) == symbolTable.getType(exp1.symID)) {
            OpStack.pop();
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private");
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_LessEqual"};
            SAS.push(newSAR);
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " <= "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }
    }
    
    void sa_GreaterEqualOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_GreaterEqualOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_GreaterEqualOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if ((symbolTable.getType(exp1.symID) == "int" || symbolTable.getType(exp1.symID) == "char")
            && symbolTable.getType(exp2.symID) == symbolTable.getType(exp1.symID)) {
            OpStack.pop();
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private");
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_GreaterEqual"};
            SAS.push(newSAR);
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " >= "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }
    }
    
    void sa_AndOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_AndOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_AndOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if (symbolTable.getType(exp1.symID) == "bool") {
            if (symbolTable.getType(exp2.symID) == "bool") {
                OpStack.pop();
                int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private");
                symbolTable.updateName(tempId);
                SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_And"};
                SAS.push(newSAR);
            }
            else {
                semanticError(exp2.lineNumber, "And requires bool found " + symbolTable.getType(exp2.symID));
            }
        }
        else {
            semanticError(exp1.lineNumber, "And requires bool found " + symbolTable.getType(exp1.symID));
        }
    }
    
    void sa_OrOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_OrOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_OrOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if (symbolTable.getType(exp1.symID) == "bool") {
            if (symbolTable.getType(exp2.symID) == "bool") {
                OpStack.pop();
                int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private");
                symbolTable.updateName(tempId);
                SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Or"};
                SAS.push(newSAR);
            }
            else {
                semanticError(exp2.lineNumber, "Or requires bool found " + symbolTable.getType(exp2.symID));
            }
        }
        else {
            semanticError(exp1.lineNumber, "Or requires bool found " + symbolTable.getType(exp1.symID));
        }
    }
    
    void sa_NotEqualOperator() {
        SAR exp1, exp2;
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_NotEqualOperator");
        else {
            exp2 = SAS.top();
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error in sa_NotEqualOperator");
        else {
            exp1 = SAS.top();
            SAS.pop();
        }
        if (symbolTable.getType(exp2.symID) == symbolTable.getType(exp1.symID)) {
            OpStack.pop();
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private");
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Less"};
            SAS.push(newSAR);
        }
        else {
            semanticError(exp2.lineNumber,
                          "Invalid Operation "
                          + symbolTable.getType(exp1.symID) + " "
                          + exp1.value + " != "
                          + symbolTable.getType(exp2.symID) + " "
                          + exp2.value);
        }
    }
    
    bool isLValue(int sid) {
        return symbolTable.getKind(sid) == "ivar" || symbolTable.getKind(sid) == "lvar";
    }
    
    void semanticAnalysis() {
        Scanner scanner(sourceCodeFilename);
        scanner.fetchTokens();  // fetch a token to nextToken
        scanner.fetchTokens();  // fetch a token to currentToken and nextToken
        compiliation_unit(scanner);
    }
    
    void run() {
//        lexicalAnalysis();
        syntaxAnalysis();
        semanticAnalysis();
    }
};

#endif /* Compiler_h */
