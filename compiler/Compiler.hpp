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
#include <vector>
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
    std::string reference;
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
    int classOffset;
    int methodOffset;
    int currentMethodId;
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
        currentMethodId = 0;
        
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
    
    std::vector<std::string> split(const std::string& s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
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
            symbolTable.insert("g", "N", symValue, "ilit", "int", "", "", "public", 0);
        }
        Token newToken = {T_Number, scanner.getToken().lineNumber, symValue};
        if (flagOfPass) sa_lPush(newToken);
    }
    
    void argument_list(Scanner& scanner) {
        expression(scanner);
        while (scanner.getToken().lexeme == ",") {
            if (flagOfPass) sa_Argument();
            scanner.fetchTokens();
            expression(scanner);
        }
    }
    
    void fn_arr_member(Scanner& scanner) {
        if (scanner.getToken().lexeme == "(") {
            if (flagOfPass) {
                sa_oPush(scanner.getToken());
                sa_BAL();
            }
            scanner.fetchTokens();
            if (scanner.getToken().lexeme != ")") {
                argument_list(scanner);
            }
            if (scanner.getToken().lexeme == ")") {
                if (flagOfPass) {
                    sa_ClosingParenthesis();
                    sa_EAL();
                    sa_func();
                }
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
        }
        else if (scanner.getToken().lexeme == "[") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
            if (scanner.getToken().lexeme == "]") {
                if (flagOfPass) {
                    sa_ClosingBracket();
                    sa_arr();
                }
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
            if (flagOfPass) {
                sa_oPush(scanner.getToken());
                sa_BAL();
            }
            scanner.fetchTokens();
            if (scanner.getToken().lexeme != ")") {
                argument_list(scanner);
            }
            if (scanner.getToken().lexeme == ")") {
                if (flagOfPass) {
                    sa_ClosingParenthesis();
                    sa_EAL();
                    sa_newObj();
                }
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
        }
        else if (scanner.getToken().lexeme == "[") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
            if (scanner.getToken().lexeme == "]") {
                if (flagOfPass) {
                    sa_ClosingBracket();
                    sa_newArray();
                }
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
                if (flagOfPass) sa_iPush(scanner.getToken());
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
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            expression(scanner);
            if (scanner.getToken().lexeme == ")") {
                if (flagOfPass) sa_ClosingParenthesis();
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
            if (flagOfPass) sa_lPush(scanner.getToken());
            scanner.fetchTokens();
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else if (scanner.getToken().lexeme == "false") {
            if (flagOfPass) sa_lPush(scanner.getToken());
            scanner.fetchTokens();
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else if (scanner.getToken().lexeme == "null") {
            if (flagOfPass) sa_lPush(scanner.getToken());
            scanner.fetchTokens();
            if (isAexpressionZ(scanner.getToken())) {
                expressionZ(scanner);
            }
        }
        else if (scanner.getToken().lexeme == "this") {
            if (flagOfPass) sa_thisPush(scanner.getToken());
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
                symbolTable.insert("g", "H", scanner.getToken().lexeme, "clit", "char", "", "", "public", 0);
            }
            if (flagOfPass) sa_lPush(scanner.getToken());
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
                symbolTable.insert("g", "H", scanner.getToken().lexeme, "clit", "char", "", "", "public", 0);
            }
            if (flagOfPass) sa_lPush(scanner.getToken());
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
        std::string labelENDIF = "";
        std::string labelSKIPELSE = "";
        std::string labelBEGIN = "";
        std::string labelENDWHILE = "";
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
                if (flagOfPass) sa_oPush(scanner.getToken());
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "(");
            }
            expression(scanner);
            if (scanner.getToken().lexeme == ")") {
                if (flagOfPass) {
                    sa_ClosingParenthesis();
                    int labelCnt = symbolTable.getNewLabelCount();
                    labelENDIF = "SKIPIF" + std::to_string(labelCnt);
                    labelSKIPELSE = "SKIPELSE" + std::to_string(labelCnt);
                    sa_if(labelENDIF);
                }
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
            statement(scanner);
            if (scanner.getToken().lexeme == "else") {
                if (flagOfPass) {
                    symbolTable.iCode(scanner.getToken().lineNumber, JMP, labelSKIPELSE, "", "", "");
                    symbolTable.iCodeLabel(labelENDIF);
                    labelENDIF = labelSKIPELSE;
                }
                scanner.fetchTokens();
                statement(scanner);
            }
            if (flagOfPass) symbolTable.iCodeLabel(labelENDIF);
        }
        else if (scanner.getToken().lexeme == "while") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme == "(") {
                if (flagOfPass) sa_oPush(scanner.getToken());
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "(");
            }
            expression(scanner);
            if (scanner.getToken().lexeme == ")") {
                if (flagOfPass) {
                    int labelCnt = symbolTable.getNewLabelCount();
                    labelBEGIN = "BEGIN" + std::to_string(labelCnt);
                    labelENDWHILE = "ENDWHILE" + std::to_string(labelCnt);
                    symbolTable.iCodeLabel(labelBEGIN);
                    sa_ClosingParenthesis();
                    sa_while(labelENDWHILE);
                }
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), ")");
            }
            statement(scanner);
            if (flagOfPass) {
                symbolTable.iCode(scanner.getToken().lineNumber, JMP, labelBEGIN, "", "", "");
                symbolTable.iCodeLabel(labelENDWHILE);
            }
        }
        else if (scanner.getToken().lexeme == "return") {
            scanner.fetchTokens();
            if (scanner.getToken().lexeme != ";") {
                expression(scanner);
            }
            if (scanner.getToken().lexeme == ";") {
                if (flagOfPass) sa_return(scanner.getToken().lineNumber);
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
                if (flagOfPass) sa_cout(scanner.getToken().lineNumber);
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
                if (flagOfPass) sa_cin(scanner.getToken().lineNumber);
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
            if (flagOfPass) sa_tPush(scanner.getToken());
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "P", paramName, "param", paramType, "", "", "private", 0);
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
                tempId = symbolTable.insert("g" + currentClass, "M", nameStr, "method", "", typeStr, "", modeStr, 0);
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
            else {
                currentMethodId = symbolTable.searchValue("g" + currentClass, nameStr);
                if (currentMethodId == 0) unexpectedError("Cannot find function symID");
                symbolTable.iCode(scanner.getToken().lineNumber, FUNC, symbolTable.getSymID(currentMethodId), "", "", symbolTable.getSymID(currentMethodId));
                methodOffset = initialMethodOffset(currentMethodId);
            }
            method_body(scanner);
            currentMethod = "";
            if (flagOfPass) {
                symbolTable.updateOffset(currentMethodId, methodOffset);
                currentMethodId = 0;
            }
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
                symbolTable.insert("g" + currentClass, "V", nameStr, "ivar", tempType, "", "", modeStr, classOffset);
                classOffset += 4;
            }
            if (scanner.getToken().lexeme == "=") {
                if (flagOfPass) sa_oPush(scanner.getToken());
                scanner.fetchTokens();
                assignment_expression(scanner);
            }
            if (scanner.getToken().lexeme == ";") {
                if (flagOfPass) sa_EOE();
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
                tempId = symbolTable.insert("g" + currentClass, "X", nameStr, "Constructor", "", nameStr, "", "public", 0);
            }
            currentMethod = "." + nameStr;
            currentParam = "";
            if (flagOfPass) sa_CD(scanner.getToken());
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
            else {
                currentMethodId = symbolTable.searchValue("g" + currentClass, nameStr);
                if (currentMethodId == 0) unexpectedError("Cannot find Constructor symID");
                symbolTable.iCode(scanner.getToken().lineNumber, FUNC, symbolTable.getSymID(currentMethodId), "", "", symbolTable.getSymID(currentMethodId));
                methodOffset = initialMethodOffset(currentMethodId);
            }
            method_body(scanner);
            currentMethod = "";
            if (flagOfPass) {
                symbolTable.updateOffset(currentMethodId, methodOffset);
                currentMethodId = 0;
            }
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
                if (flagOfPass) sa_tPush(scanner.getToken());
                typeStr = scanner.getToken().lexeme;
                scanner.fetchTokens();
            }
            else {
                syntaxError(scanner.getToken(), "type");
            }
            if (scanner.getToken().type == T_Identifier) {
                if (flagOfPass) sa_vPush(scanner.getToken());
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
        classOffset = 0;
        int classId = 0;
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
                    classId = symbolTable.insert("g", "C", scanner.getToken().lexeme, "Class", "", "", "", "", 0);
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
        if (!flagOfPass)
            symbolTable.updateOffset(classId, classOffset);
    }
    
    void variable_declaration(Scanner & scanner) {
        std::string typeStr;
        std::string nameStr;
        if (isAtype(scanner.getToken())) {
            if (flagOfPass) sa_tPush(scanner.getToken());
            typeStr = scanner.getToken().lexeme;
            scanner.fetchTokens();
        }
        else {
            syntaxError(scanner.getToken(), "type");
        }
        if (scanner.getToken().type == T_Identifier) {
            nameStr = scanner.getToken().lexeme;
            //#dup
            if (!flagOfPass && symbolTable.searchValue("g" + currentClass + currentMethod, nameStr) != 0) {
                semanticError(scanner.getToken().lineNumber, "Duplicate variable " + nameStr);
            }
            if (flagOfPass) sa_vPush(scanner.getToken());
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
            symbolTable.insert("g" + currentClass + currentMethod, "L", nameStr, "lvar", typeStr, "", "", "private", 0);
        }
        else {
            int tempId = symbolTable.searchValue("g" + currentClass + currentMethod, nameStr);
            if (tempId == 0) unexpectedError("Cannot find variable symID");
            symbolTable.updateOffset(tempId, methodOffset);
            methodOffset += 4;
        }
        if (scanner.getToken().lexeme == "=") {
            if (flagOfPass) sa_oPush(scanner.getToken());
            scanner.fetchTokens();
            assignment_expression(scanner);
        }
        if (scanner.getToken().lexeme == ";") {
            if (flagOfPass) sa_EOE();
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
        if (flagOfPass) symbolTable.iCode(scanner.getToken().lineNumber, RTN, "", "", "", "");
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
            if (flagOfPass) {
                currentMethodId = symbolTable.searchValue("g.main", "main");
                if (currentMethodId == 0) unexpectedError("Cannot find main function symID");
                symbolTable.iCode(scanner.getToken().lineNumber, FUNC, symbolTable.getSymID(currentMethodId), "", "", symbolTable.getSymID(currentMethodId));
                methodOffset = 12;
            }
            currentClass = ".main";
            currentMethod = ".main";
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
        currentMethod = "";
        if (flagOfPass) {
            symbolTable.updateOffset(currentMethodId, methodOffset);
            currentMethodId = 0;
        }
    }
    
    int initialMethodOffset(int methodId) {
        int result = 12;
        std::string paramStr = symbolTable.getParam(methodId);
        if (paramStr.size() > 2) {
            std::vector<std::string> paramList = split(paramStr.substr(1,paramStr.size() - 2), ',');
            for (int i = 0; i < paramList.size(); i++) {
                symbolTable.updateOffset(stoi(paramList[i].substr(1, paramList[i].size() - 1)), result);
                result += 4;
            }
        }
        return result;
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
    }
    
    void sa_iPush(Token token) {
        SAR newSAR = {0, token.lineNumber, "id_sar", token.lexeme, "sa_iPush"};
        SAS.push(newSAR);
    }
    
    void sa_lPush(Token token) {
        int tempId = symbolTable.searchValue("g", token.lexeme);
        if (tempId == 0) {
            semanticError(token.lineNumber, "Unexpected error in sa_lPush");
        }
        SAR newSAR = {tempId, token.lineNumber, "lit_sar", token.lexeme, "sa_lPush"};
        SAS.push(newSAR);
    }
    
    void sa_oPush(Token token) {
        if (token.lexeme == "=" && !OpStack.empty())
            semanticError(token.lineNumber, "Assignment operation error");
        else if (token.lexeme == "(" || token.lexeme == "[") {
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
    
    void sa_tPush(Token token) {
        SAR newSAR = {0, token.lineNumber, "type_sar", token.lexeme, "sa_tPush"};
        if (token.lexeme != "int" && token.lexeme != "bool" && token.lexeme != "char" && token.lexeme != "void" && token.lexeme != "sym") {
            // #tExist
            int tempId = symbolTable.searchValue("g", token.lexeme);
            if (symbolTable.getKind(tempId) != "Class")
                semanticError(token.lineNumber, "Type \"" + token.lexeme + "\" not defined");
            else
                newSAR.symID = tempId;
        }
        SAS.push(newSAR);
    }
    
    void sa_thisPush(Token token) {
        if (token.lexeme != "this") {
            unexpectedError("Unexpected access function sa_thisPush");
        }
        if (currentClass == ".main") {
            semanticError(token.lineNumber, "Variable \""  + token.lexeme + "\" not defined in \"main\"");
        }
        SAR newSAR = {0, token.lineNumber, "this_sar", token.lexeme, "sa_thisPush"};
        SAS.push(newSAR);
    }
    
    void sa_iExist() {
        if (SAS.empty()) unexpectedError("SAS is empty -- #iExist");
        if (SAS.top().reference == "id_sar") {
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
        else if (SAS.top().reference == "func_sar") {
            int tempId = 0;
            SAR topSAR = SAS.top();
            std::string currentPath = "g" + currentClass + currentMethod;
            std::string funcName = topSAR.value.substr(0, topSAR.value.find_first_of('('));
            while (currentPath != "g") {
                tempId = symbolTable.searchValue(currentPath, funcName);
                if (tempId != 0) break;
                currentPath = currentPath.substr(0, currentPath.find_last_of('.'));
            }
            if (tempId != 0 && symbolTable.getKind(tempId) == "method") {
                std::string funcSignature = symbolTable.getValue(tempId);
                std::string paramStr = symbolTable.getParam(tempId);
                if (paramStr.size() <= 2)
                    funcSignature += "()";
                else {
                    std::vector<std::string> pList = split(paramStr.substr(1,paramStr.size() - 2), ',');
                    funcSignature += "(";
                    for (int i = 0; i < pList.size(); i++) {
                        funcSignature += symbolTable.getType(stoi(pList[i].substr(1, pList[i].size() - 1)));
                        funcSignature += ",";
                    }
                    funcSignature[funcSignature.size() - 1] = ')';
                }
                std::vector<std::string> paramList;
                std::string topSARparam = topSAR.value.substr(topSAR.value.find_first_of('('));
                std::string topSARsignature = topSAR.value.substr(0, topSAR.value.find_first_of('('));
                if (topSARparam.size() <= 2)
                    topSARsignature += "()";
                else {
                    paramList = split(topSARparam.substr(1,topSARparam.size() - 2), ',');
                    topSARsignature += "(";
                    for (int i = 0; i < paramList.size(); i++) {
                        topSARsignature += symbolTable.getType(stoi(paramList[i].substr(1, paramList[i].size() - 1)));
                        topSARsignature += ",";
                    }
                    topSARsignature[topSARsignature.size() - 1] = ')';
                }

                if (topSARsignature != funcSignature) {
                    semanticError(topSAR.lineNumber, "Function \""  + topSARsignature + "\" not defined");
                }
                else {
                    int newId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", symbolTable.getReturnType(tempId), "", "", "private", methodOffset);
                    methodOffset += 4;
                    SAR newSAR = {newId, topSAR.lineNumber, "ref_sar", topSARsignature, "sa_iExist"};
                    SAS.pop();
                    SAS.push(newSAR);

                    symbolTable.iCode(topSAR.lineNumber, FRAME, symbolTable.getSymID(tempId), "this", "", "");
                    for (int i = 0; i < paramList.size(); i++) {
                        symbolTable.iCode(topSAR.lineNumber, PUSH, paramList[i], "", "", "");
                    }
                    symbolTable.iCode(topSAR.lineNumber, CALL, symbolTable.getSymID(tempId), "", "", "");
                    if (symbolTable.getReturnType(tempId) != "void") {
                        symbolTable.iCode(topSAR.lineNumber, PEEK, symbolTable.getSymID(newId), "", "", "");
                    }
                }
            }
            else {
                semanticError(SAS.top().lineNumber, "Function \"" + SAS.top().value + "\" not defined");
            }
        }
        else if (SAS.top().reference == "arr_sar") {
            int tempId = 0;
            std::string currentPath = "g" + currentClass + currentMethod;
            while (currentPath != "g") {
                tempId = symbolTable.searchValue(currentPath, SAS.top().value);
                if (tempId != 0) break;
                currentPath = currentPath.substr(0, currentPath.find_last_of('.'));
            }
            if (tempId != 0 && symbolTable.getType(tempId).size() > 0 && symbolTable.getType(tempId)[0] == '@') {
                int newId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "lvar", symbolTable.getType(tempId).substr(2), "", "", "private", methodOffset);
                methodOffset += 4;
                SAR newSAR = {newId, SAS.top().lineNumber, "id_sar", symbolTable.getSymID(newId), "sa_iExist"};
                //use SAS.top().symID to find the index of array
                symbolTable.iCode(SAS.top().lineNumber, AEF, symbolTable.getSymID(tempId), symbolTable.getSymID(SAS.top().symID), symbolTable.getSymID(newId), "");
                SAS.pop();
                SAS.push(newSAR);
            }
            else {
                semanticError(SAS.top().lineNumber, "Array \"" + SAS.top().value + "\" not defined");
            }
        }
        else
            semanticError(SAS.top().lineNumber, "unknown SAR " + SAS.top().value);
    }
    
    void sa_vPush(Token token) {
        int tempId = symbolTable.searchValue("g" + currentClass + currentMethod, token.lexeme);
        if (tempId == 0) {
            semanticError(token.lineNumber, "Unexpected error in sa_vPush");
        }
        SAR newSAR = {tempId, token.lineNumber, "variable", token.lexeme, "sa_vPush"};
        SAS.push(newSAR);
    }
    
    void sa_rExist() {
        if (SAS.empty()) unexpectedError("SAS is empty -- #rExist");
        SAR topSAR = SAS.top();
        SAS.pop();
        if (SAS.empty()) unexpectedError("SAS is empty -- #rExist");
        SAR nextSAR = SAS.top();
        SAS.pop();
        
        if (topSAR.reference == "id_sar") {
            int classID = 0;
            if (nextSAR.value == "this") {
                classID = symbolTable.searchValue("g", currentClass.substr(1));
            }
            else
                classID = symbolTable.getClassIDFromObject(nextSAR.symID);
            if (classID == 0) {
                semanticError(topSAR.lineNumber, "Variable \""  + topSAR.value + "\" not defined/public in class \"" + nextSAR.value + "\"");
            }
            int tempId = symbolTable.searchValue("g." + symbolTable.getValue(classID), topSAR.value);
            if (tempId == 0 || symbolTable.getKind(tempId) != "ivar" ||
                (nextSAR.value != "this" && symbolTable.getAccessMod(tempId) != "public")) {
                semanticError(topSAR.lineNumber, "Variable \""  + topSAR.value + "\" not defined/public in class \"" + nextSAR.value + "\"");
            }
            else {
                int newId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "lvar", symbolTable.getType(tempId), "", "", "private", methodOffset);
                methodOffset += 4;
                SAR newSAR = {newId, topSAR.lineNumber, "ref_sar", nextSAR.value + "." + topSAR.value, "sa_rExist"};
                SAS.push(newSAR);

                symbolTable.iCode(nextSAR.lineNumber, REF, symbolTable.getSymID(nextSAR.symID), symbolTable.getSymID(tempId), symbolTable.getSymID(newId), "");
            }
        }
        else if (topSAR.reference == "func_sar") {
            int classID = 0;
            if (nextSAR.value == "this") {
                classID = symbolTable.searchValue("g", currentClass.substr(1));
            }
            else
                classID = symbolTable.getClassIDFromObject(nextSAR.symID);
            if (classID == 0) {
                semanticError(topSAR.lineNumber, "Function \""  + topSAR.value + "\" not defined/public in class \"" + nextSAR.value + "\"");
            }
            std::string funcName = topSAR.value.substr(0, topSAR.value.find_first_of('('));
            int tempId = symbolTable.searchValue("g." + symbolTable.getValue(classID), funcName);
            if (tempId == 0 || symbolTable.getKind(tempId) != "method" ||
                (nextSAR.value != "this" && symbolTable.getAccessMod(tempId) != "public")) {
                semanticError(topSAR.lineNumber, "Function \""  + topSAR.value + "\" not defined/public in class \"" + nextSAR.value + "\"");
            }
            std::string funcSignature = symbolTable.getValue(tempId);
            std::string paramStr = symbolTable.getParam(tempId);
            if (paramStr.size() <= 2)
                funcSignature += "()";
            else {
                std::vector<std::string> pList = split(paramStr.substr(1,paramStr.size() - 2), ',');
                funcSignature += "(";
                for (int i = 0; i < pList.size(); i++) {
                    funcSignature += symbolTable.getType(stoi(pList[i].substr(1, pList[i].size() - 1)));
                    funcSignature += ",";
                }
                funcSignature[funcSignature.size() - 1] = ')';
            }
            std::vector<std::string> paramList;
            std::string topSARparam = topSAR.value.substr(topSAR.value.find_first_of('('));
            std::string topSARsignature = topSAR.value.substr(0, topSAR.value.find_first_of('('));
            if (topSARparam.size() <= 2)
                topSARsignature += "()";
            else {
                paramList = split(topSARparam.substr(1,topSARparam.size() - 2), ',');
                topSARsignature += "(";
                for (int i = 0; i < paramList.size(); i++) {
                    topSARsignature += symbolTable.getType(stoi(paramList[i].substr(1, paramList[i].size() - 1)));
                    topSARsignature += ",";
                }
                topSARsignature[topSARsignature.size() - 1] = ')';
            }
            
            if (topSARsignature != funcSignature) {
                semanticError(topSAR.lineNumber, "Function \""  + topSARsignature + "\" not defined/public in class \"" + nextSAR.value + "\"");
            }
            else {
                int newId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", symbolTable.getReturnType(tempId), "", "", "private", methodOffset);
                methodOffset += 4;
                SAR newSAR = {newId, topSAR.lineNumber, "ref_sar", nextSAR.value + "." + topSARsignature, "sa_rExist"};
                SAS.push(newSAR);

                symbolTable.iCode(nextSAR.lineNumber, FRAME, symbolTable.getSymID(tempId), symbolTable.getSymID(nextSAR.symID), "", "");
                for (int i = 0; i < paramList.size(); i++) {
                    symbolTable.iCode(nextSAR.lineNumber, PUSH, paramList[i], "", "", "");
                }
                symbolTable.iCode(nextSAR.lineNumber, CALL, symbolTable.getSymID(tempId), "", "", "");
                if (symbolTable.getReturnType(tempId) != "void") {
                    symbolTable.iCode(nextSAR.lineNumber, PEEK, symbolTable.getSymID(newId), "", "", "");
                }
            }
        }
        else if (topSAR.reference == "arr_sar") {
            int classID = 0;
            if (nextSAR.value == "this")
                classID = symbolTable.searchValue("g", currentClass.substr(1));
            else
                classID = symbolTable.getClassIDFromObject(nextSAR.symID);
            if (classID == 0) {
                semanticError(topSAR.lineNumber, "Array \""  + topSAR.value + "\" not defined/public in class \"" + nextSAR.value + "\"");
            }
            int tempId = symbolTable.searchValue("g." + symbolTable.getValue(classID), topSAR.value);
            if (tempId == 0 || symbolTable.getType(tempId).size() == 0 || symbolTable.getType(tempId)[0] != '@' ||
                (nextSAR.value != "this" && symbolTable.getAccessMod(tempId) != "public")) {
                semanticError(topSAR.lineNumber, "Array \"" + topSAR.value + "\" not defined/public in class \"" + nextSAR.value + "\"");
            }
            else {
                int refId = symbolTable.insert("g" + currentClass + currentMethod, "R", "", "tvar", symbolTable.getType(tempId), "", "", "private", methodOffset);
                methodOffset += 4;
                int newId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "lvar", symbolTable.getType(tempId).substr(2), "", "", "private", methodOffset);
                methodOffset += 4;
                SAR newSAR = {newId, topSAR.lineNumber, "id_sar", symbolTable.getSymID(newId), "sa_rExist"};
                SAS.push(newSAR);
                
                symbolTable.iCode(nextSAR.lineNumber, REF, symbolTable.getSymID(nextSAR.symID), symbolTable.getSymID(tempId), symbolTable.getSymID(refId), "");
                //use topSAR.symID to find the index of array
                symbolTable.iCode(nextSAR.lineNumber, AEF, symbolTable.getSymID(refId), symbolTable.getSymID(topSAR.symID), symbolTable.getSymID(newId), "");
            }
        }
        else
            semanticError(topSAR.lineNumber, "unknown SAR " + topSAR.value);
    }
    
    
    void sa_BAL() {
        SAR newSAR = {0, 0, "bal_sar", "", "sa_BAL"};
        SAS.push(newSAR);
    }
    
    void sa_EAL() {
        std::string paramList = "(";
        std::stack<std::string> paramType;
        while (!SAS.empty() && SAS.top().reference != "bal_sar") {
            paramType.push(symbolTable.getSymID(SAS.top().symID));
            SAS.pop();
        }
        if (SAS.empty())
            semanticError(0, "Unexpected Error on sa_BAL");
        else
            SAS.pop(); // remove bal_sar
        if (paramType.empty())
            paramList += ")";
        else {
            while (!paramType.empty()) {
                paramList += paramType.top() + ",";
                paramType.pop();
            }
            paramList[paramList.size() - 1] = ')'; //substitute the last ',' to ')'
        }
        SAR newSAR = {0, 0, "al_sar", paramList, "sa_EAL"};
        SAS.push(newSAR);
    }
    
    void sa_func() {
        if (SAS.empty()) unexpectedError("SAS is empty -- #sa_func");
        SAR alSAR = SAS.top();
        SAS.pop();
        if (SAS.empty()) unexpectedError("SAS is empty -- #sa_func");
        SAR nextSAR = SAS.top();
        SAS.pop();
        
        SAR newSAR = {0, nextSAR.lineNumber, "func_sar", nextSAR.value + alSAR.value, "sa_func"};
        SAS.push(newSAR);
    }
    
    void sa_arr() {
        if (SAS.empty()) unexpectedError("SAS is empty -- #sa_arr");
        SAR arrIndex = SAS.top();
        SAS.pop();
        if (SAS.empty()) unexpectedError("SAS is empty -- #sa_arr");
        SAR idSAR = SAS.top();
        SAS.pop();

        if (symbolTable.getType(arrIndex.symID) != "int") {
            semanticError(idSAR.lineNumber, "Array requires int index got " + symbolTable.getType(arrIndex.symID));
        }
        
        SAR newSAR = {arrIndex.symID, idSAR.lineNumber, "arr_sar", idSAR.value, "sa_arr"};
        SAS.push(newSAR);
    }
    
    void sa_if(std::string label) {
        if (SAS.empty()) unexpectedError("Unexpected Error: SAS empty in sa_if()");
        if (symbolTable.getType(SAS.top().symID) == "bool") {
            symbolTable.iCode(SAS.top().lineNumber, BF, symbolTable.getSymID(SAS.top().symID), label, "", "");
            SAS.pop();
        }
        else
            semanticError(SAS.top().lineNumber, "'if' requires 'bool' got \'" + symbolTable.getType(SAS.top().symID) + "\'");
    }
    
    void sa_while(std::string label) {
        if (SAS.empty()) unexpectedError("Unexpected Error: SAS empty in sa_while()");
        if (symbolTable.getType(SAS.top().symID) == "bool") {
            symbolTable.iCode(SAS.top().lineNumber, BF, symbolTable.getSymID(SAS.top().symID), label, "", "");
            SAS.pop();
        }
        else
            semanticError(SAS.top().lineNumber, "'while' requires 'bool' got \'" + symbolTable.getType(SAS.top().symID) + "\'");
    }
    
    void sa_return(int lineNumber) {
        int tempId = symbolTable.searchValue("g" + currentClass, currentMethod.substr(1));
        if (tempId == 0) {
            unexpectedError("Unexpected Error: function id not found");
        }
        else {
            while (!OpStack.empty()) {
                if (OpStack.top().value == "=") {
                    sa_AssigmentOperator();
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
                    semanticError(0, "Unexpected Error on sa_return");
                }
            }
            std::string expressionType;
            if (SAS.empty()) {
                expressionType = "void";
                symbolTable.iCode(lineNumber, RTN, "", "", "", "");
            }
            else {
                expressionType = symbolTable.getType(SAS.top().symID);
                symbolTable.iCode(lineNumber, RETURN, symbolTable.getSymID(SAS.top().symID), "", "", "");
            }
            if (expressionType != symbolTable.getReturnType(tempId)) {
                semanticError(lineNumber, "Function requires \"" + symbolTable.getReturnType(tempId) + "\" returned \"" + expressionType + "\"");
            }
            while (!SAS.empty()) SAS.pop();
        }
    }
    
    void sa_cout(int lineNumber) {
        while (!OpStack.empty()) {
            if (OpStack.top().value == "=") {
                sa_AssigmentOperator();
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
                semanticError(0, "Unexpected Error on sa_cout");
            }
        }
        std::string expressionType;
        if (SAS.empty()) {
            expressionType = "void";
        }
        else {
            expressionType = symbolTable.getType(SAS.top().symID);
        }
        if (expressionType == "char") {
            symbolTable.iCode(SAS.top().lineNumber, WRTC, symbolTable.getSymID(SAS.top().symID), "", "", "");
        }
        else if (expressionType == "int") {
            symbolTable.iCode(SAS.top().lineNumber, WRTI, symbolTable.getSymID(SAS.top().symID), "", "", "");
        }
        else if (expressionType == "bool") {
            symbolTable.iCode(SAS.top().lineNumber, WRITE, symbolTable.getSymID(SAS.top().symID), "", "", "");
        }
        else
            semanticError(lineNumber, "cout not defined for " + expressionType);
        while (!SAS.empty()) SAS.pop();
    }
    
    void sa_cin(int lineNumber) {
        while (!OpStack.empty()) {
            if (OpStack.top().value == "=") {
                sa_AssigmentOperator();
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
                semanticError(0, "Unexpected Error on sa_cin");
            }
        }
        std::string expressionType;
        if (SAS.empty()) {
            expressionType = "void";
        }
        else {
            expressionType = symbolTable.getType(SAS.top().symID);
        }
        if ((expressionType == "char" || expressionType == "int") &&
            !SAS.empty() && isLValue(SAS.top().symID)) {
            if (expressionType == "char") {
                symbolTable.iCode(SAS.top().lineNumber, RDC, symbolTable.getSymID(SAS.top().symID), "", "", "");
            }
            else if (expressionType == "int") {
                symbolTable.iCode(SAS.top().lineNumber, RDI, symbolTable.getSymID(SAS.top().symID), "", "", "");
            }
        }
        else
            semanticError(lineNumber, "cin not defined for " + expressionType);
        while (!SAS.empty()) SAS.pop();
    }
    
    void sa_newObj() {
        if (SAS.empty()) unexpectedError("SAS is empty -- #sa_newObj");
        SAR alSAR = SAS.top();
        SAS.pop();
        if (SAS.empty()) unexpectedError("SAS is empty -- #sa_newObj");
        SAR typeSAR = SAS.top();
        SAS.pop();
        
        int tempId = symbolTable.searchValue("g." + typeSAR.value, typeSAR.value);
        if (tempId == 0 || symbolTable.getKind(tempId) != "Constructor" || symbolTable.getAccessMod(tempId) != "public") {
            semanticError(typeSAR.lineNumber, "Constructor \""  + typeSAR.value + alSAR.value + "\" not defined");
        }
        std::string funcSignature = typeSAR.value;
        std::string paramStr = symbolTable.getParam(tempId);
        
        // todo: change the parameter calculation
        if (paramStr.size() <= 2)
            funcSignature += "()";
        else {
            std::vector<std::string> paramList = split(paramStr.substr(1,paramStr.size() - 2), ',');
            funcSignature += "(";
            for (int i = 0; i < paramList.size(); i++) {
                funcSignature += symbolTable.getType(stoi(paramList[i].substr(1, paramList[i].size() - 1)));
                funcSignature += ",";
            }
            funcSignature[funcSignature.size() - 1] = ')';
        }
        if (funcSignature != typeSAR.value + alSAR.value) {
            semanticError(typeSAR.lineNumber, "Constructor \""  + typeSAR.value + alSAR.value + "\" not defined");
        }
        else {
            int newId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", typeSAR.value, "", "", "private", methodOffset);
            methodOffset += 4;
            SAR newSAR = {newId, typeSAR.lineNumber, "new_sar", typeSAR.value + alSAR.value, "sa_newObj"};
            SAS.push(newSAR);
        }
    }
    
    void sa_newArray() {
        if (SAS.empty()) unexpectedError("SAS is empty -- #sa_newArray");
        SAR arrIndex = SAS.top();
        SAS.pop();
        if (SAS.empty()) unexpectedError("SAS is empty -- #sa_newArray");
        SAR typeSAR = SAS.top();
        SAS.pop();
        
        if (symbolTable.getType(arrIndex.symID) != "int") {
            semanticError(typeSAR.lineNumber, "Array requires int index got " + symbolTable.getType(arrIndex.symID));
        }
        
        if (typeSAR.value == "int" || typeSAR.value == "bool" || typeSAR.value == "char" || symbolTable.searchValue("g", typeSAR.value) != 0) {
            int newId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "@:" + typeSAR.value, "", "", "private", methodOffset);
            methodOffset += 4;
            SAR newSAR = {newId, typeSAR.lineNumber, "new_sar", "", "sa_newArray"};
            SAS.push(newSAR);
        }
        else {
            semanticError(typeSAR.lineNumber, "Type \""  + typeSAR.value + "\" not defined");
        }
    }
    
    void sa_CD(Token token) {
        if (token.lexeme != currentClass.substr(1))
            semanticError(token.lineNumber, "Constructor \"" + token.lexeme + "\" must match class name \"" + currentClass.substr(1) + "\"");
    }
    
    void sa_ClosingParenthesis() {
        while (!OpStack.empty() && OpStack.top().value != "(") {
            if (OpStack.top().value == "=") {
                sa_AssigmentOperator();
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
                semanticError(0, "Unexpected Error on sa_ClosingParenthesis");
            }
        }
        if (OpStack.empty())
            semanticError(0, "Unexpected Error on sa_ClosingParenthesis");
        else
            OpStack.pop(); //remove the beginning parenthesis
    }
    
    void sa_ClosingBracket() {
        while (!OpStack.empty() && OpStack.top().value != "[") {
            if (OpStack.top().value == "=") {
                sa_AssigmentOperator();
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
                semanticError(0, "Unexpected Error on sa_ClosingBracket");
            }
        }
        if (OpStack.empty())
            semanticError(0, "Unexpected Error on sa_ClosingBracket");
        else
            OpStack.pop(); //remove the beginning Bracket
    }
    
    void sa_Argument() {
        while (!OpStack.empty() && OpStack.top().value != "(") {
            if (OpStack.top().value == "=") {
                sa_AssigmentOperator();
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
                semanticError(0, "Unexpected Error on sa_Argument");
            }
        }
        if (OpStack.empty()) semanticError(0, "Unexpected Error on sa_Argument");
    }
    
    void sa_EOE() {
        while (!OpStack.empty()) {
            if (OpStack.top().value == "=") {
                sa_AssigmentOperator();
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
        while (!SAS.empty()) SAS.pop();
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "int", "", "", "private", methodOffset);
            methodOffset += 4;
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Add"};
            SAS.push(newSAR);

            symbolTable.iCode(exp1.lineNumber, ADD, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "int", "", "", "private", methodOffset);
            methodOffset += 4;
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Subtract"};
            SAS.push(newSAR);
 
            symbolTable.iCode(exp1.lineNumber, SUB, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "int", "", "", "private", methodOffset);
            methodOffset += 4;
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Multiply"};
            SAS.push(newSAR);
            
            symbolTable.iCode(exp1.lineNumber, MUL, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "int", "", "", "private", methodOffset);
            methodOffset += 4;
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Divide"};
            SAS.push(newSAR);
            
            symbolTable.iCode(exp1.lineNumber, DIV, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
    
    void sa_AssigmentOperator() {
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
        if (isLValue(exp1.symID) &&
            (symbolTable.getType(exp1.symID) == symbolTable.getType(exp2.symID) || (symbolTable.getType(exp2.symID) == "null"))) {
            OpStack.pop();
            
            symbolTable.iCode(exp1.lineNumber, MOV, symbolTable.getSymID(exp2.symID), symbolTable.getSymID(exp1.symID), "", "");
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private", methodOffset);
            methodOffset += 4;
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Less"};
            SAS.push(newSAR);
            
            symbolTable.iCode(exp1.lineNumber, LT, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private", methodOffset);
            methodOffset += 4;
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Greater"};
            SAS.push(newSAR);
            
            symbolTable.iCode(exp1.lineNumber, GT, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private", methodOffset);
            methodOffset += 4;
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Equal"};
            SAS.push(newSAR);
            
            symbolTable.iCode(exp1.lineNumber, EQ, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private", methodOffset);
            methodOffset += 4;
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_LessEqual"};
            SAS.push(newSAR);
            
            symbolTable.iCode(exp1.lineNumber, LE, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private", methodOffset);
            methodOffset += 4;
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_GreaterEqual"};
            SAS.push(newSAR);
            
            symbolTable.iCode(exp1.lineNumber, GE, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
                int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private", methodOffset);
                methodOffset += 4;
                symbolTable.updateName(tempId);
                SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_And"};
                SAS.push(newSAR);
                
                symbolTable.iCode(exp1.lineNumber, AND, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
                int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private", methodOffset);
                methodOffset += 4;
                symbolTable.updateName(tempId);
                SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Or"};
                SAS.push(newSAR);
                
                symbolTable.iCode(exp1.lineNumber, OR, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
            int tempId = symbolTable.insert("g" + currentClass + currentMethod, "T", "", "tvar", "bool", "", "", "private", methodOffset);
            methodOffset += 4;
            symbolTable.updateName(tempId);
            SAR newSAR = {tempId, exp1.lineNumber, "tvar_sar", symbolTable.getValue(tempId), "sa_Less"};
            SAS.push(newSAR);
            
            symbolTable.iCode(exp1.lineNumber, NE, symbolTable.getSymID(exp1.symID), symbolTable.getSymID(exp2.symID), symbolTable.getSymID(tempId), "");
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
//        std::cout << "Semantic Check Passed\n";
    }
    
    void run() {
//        lexicalAnalysis();
        syntaxAnalysis();
        semanticAnalysis();
//        symbolTable.printAll();
        symbolTable.printICode();
//        symbolTable.generateTCode();
    }
};

#endif /* Compiler_h */
