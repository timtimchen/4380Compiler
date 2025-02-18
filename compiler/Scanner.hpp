//
//  scanner.hpp
//  compiler
//
//  Created by jing hong chen on 1/19/19.
//  Copyright © 2019 jing hong chen. All rights reserved.
//

#ifndef Scanner_h
#define Scanner_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <iterator>

enum TokenType {
    T_Number,
    T_Character,
    T_Identifier,
    T_Punctuation,
    T_Keywords,
    T_Symbols,
    T_Unknown,
    T_EOF
};

enum CharGroup {
    C_Whitespace,
    C_Number,
    C_Letter,
    C_Apostrophe,
    C_Backslash,
    C_Slash,
    C_ExclamationMark,
    C_Symbol,
    C_Punctuation,
    C_UnGroup
};

struct Token {
    TokenType type;
    int lineNumber;
    std::string lexeme;
};

class Scanner {
private:
    std::ifstream inputFile;
    Token currentToken, nextToken;
    int lineIndex;
    int charIndex;
    std::string lineBuffer;
    bool endLineFlag;
    std::map<std::string, int> keywordTable;
    std::map<std::string, int> twoCharSymbols;
    
public:
    Scanner(std::string filename) {
        inputFile.open(filename);
        if (!inputFile.is_open()) {
            std::cout << "Cannot open the file: " << filename << std::endl;
            exit(1);
        }

        currentToken = { T_EOF, 0, "" };
        nextToken = { T_EOF, 0, "" };
        lineIndex = 0;
        charIndex = 0;
        endLineFlag = true;
        
        keywordTable.insert(std::pair<std::string, int>("atoi",0));
        keywordTable.insert(std::pair<std::string, int>("and",0));
        keywordTable.insert(std::pair<std::string, int>("bool",0));
        keywordTable.insert(std::pair<std::string, int>("block",0));
        keywordTable.insert(std::pair<std::string, int>("break",0));
        keywordTable.insert(std::pair<std::string, int>("case",0));
        keywordTable.insert(std::pair<std::string, int>("class",0));
        keywordTable.insert(std::pair<std::string, int>("char",0));
        keywordTable.insert(std::pair<std::string, int>("cin",0));
        keywordTable.insert(std::pair<std::string, int>("cout",0));
        keywordTable.insert(std::pair<std::string, int>("default",0));
        keywordTable.insert(std::pair<std::string, int>("else",0));
        keywordTable.insert(std::pair<std::string, int>("false",0));
        keywordTable.insert(std::pair<std::string, int>("if",0));
        keywordTable.insert(std::pair<std::string, int>("int",0));
        keywordTable.insert(std::pair<std::string, int>("itoa",0));
        keywordTable.insert(std::pair<std::string, int>("kxi2019",0));
        keywordTable.insert(std::pair<std::string, int>("lock",0));
        keywordTable.insert(std::pair<std::string, int>("main",0));
        keywordTable.insert(std::pair<std::string, int>("new",0));
        keywordTable.insert(std::pair<std::string, int>("null",0));
        keywordTable.insert(std::pair<std::string, int>("object",0));
        keywordTable.insert(std::pair<std::string, int>("or",0));
        keywordTable.insert(std::pair<std::string, int>("public",0));
        keywordTable.insert(std::pair<std::string, int>("private",0));
        keywordTable.insert(std::pair<std::string, int>("protected",0));
        keywordTable.insert(std::pair<std::string, int>("return",0));
        keywordTable.insert(std::pair<std::string, int>("release",0));
        keywordTable.insert(std::pair<std::string, int>("string",0));
        keywordTable.insert(std::pair<std::string, int>("spawn",0));
        keywordTable.insert(std::pair<std::string, int>("sym",0));
        keywordTable.insert(std::pair<std::string, int>("set",0));
        keywordTable.insert(std::pair<std::string, int>("switch",0));
        keywordTable.insert(std::pair<std::string, int>("this",0));
        keywordTable.insert(std::pair<std::string, int>("true",0));
        keywordTable.insert(std::pair<std::string, int>("thread",0));
        keywordTable.insert(std::pair<std::string, int>("unprotected",0));
        keywordTable.insert(std::pair<std::string, int>("unlock",0));
        keywordTable.insert(std::pair<std::string, int>("void",0));
        keywordTable.insert(std::pair<std::string, int>("while",0));
        keywordTable.insert(std::pair<std::string, int>("wait",0));
        
        twoCharSymbols.insert(std::pair<std::string, int>("<<",1));
        twoCharSymbols.insert(std::pair<std::string, int>(">>",1));
        twoCharSymbols.insert(std::pair<std::string, int>("&&",0));
        twoCharSymbols.insert(std::pair<std::string, int>("||",0));
        twoCharSymbols.insert(std::pair<std::string, int>("==",0));
        twoCharSymbols.insert(std::pair<std::string, int>("<=",0));
        twoCharSymbols.insert(std::pair<std::string, int>(">=",0));
    }
    
    ~Scanner() {
        inputFile.close();
    }

    CharGroup parseChar(char ch) {
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f') return C_Whitespace;
        if (ch >= '0' && ch <= '9') return C_Number;
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) return C_Letter;
        if (ch == '\'') return C_Apostrophe;
        if (ch == '\\') return C_Backslash;
        if (ch == '/') return C_Slash;
        if (ch == '!') return C_ExclamationMark;
        if (ch == '='
            || ch == '+'
            || ch == '-'
            || ch == '*'
            || ch == '('
            || ch == ')'
            || ch == '<'
            || ch == '>'
            || ch == '['
            || ch == ']'
            || ch == '{'
            || ch == '}'
            || ch == '|'
            || ch == '&')
        return C_Symbol;
        if (ch == '.' || ch == ',' || ch == ';' || ch == ':') return C_Punctuation;
        return C_UnGroup;
    }
    
    // set new currentToken, and fecth a new token from the source file, if the file is end, set nextToken as a TOF
    void fetchTokens() {
        currentToken = nextToken;
        
        while (true) {
            if (endLineFlag || lineBuffer.empty() || charIndex >= lineBuffer.size()) {
                lineIndex++;
                if (std::getline(inputFile, lineBuffer)) {
                    charIndex = 0;
                    endLineFlag = false;
                }
                else {
                    nextToken = { T_EOF, lineIndex, "EOF" };
                    return;
                }
            }
            else {
                if (parseChar(lineBuffer[charIndex]) == C_Whitespace) {
                    charIndex++;
                }
                else {
                    std::string tempStr = "";
                    switch (parseChar(lineBuffer[charIndex])) {
                        case C_Number:
                        while (charIndex < lineBuffer.size() && parseChar(lineBuffer[charIndex]) == C_Number) {
                            tempStr += lineBuffer[charIndex];
                            charIndex++;
                        }
                        nextToken = { T_Number, lineIndex, tempStr};
                        return;
                        case C_Letter:
                        while (charIndex < lineBuffer.size()
                               && (parseChar(lineBuffer[charIndex]) == C_Number
                                   || parseChar(lineBuffer[charIndex]) == C_Letter)) {
                            tempStr += lineBuffer[charIndex];
                            charIndex++;
                        }
                        if (keywordTable.find(tempStr) == keywordTable.end()) {
                            nextToken = { T_Identifier, lineIndex, tempStr};
                        }
                        else {
                            nextToken = { T_Keywords, lineIndex, tempStr};
                        }
                        return;
                        case C_Apostrophe:
                        tempStr += '\'';
                        charIndex++;
                        if (charIndex < lineBuffer.size()
                            && parseChar(lineBuffer[charIndex]) == C_Backslash
                            && charIndex + 2 < lineBuffer.size()
                            && lineBuffer[charIndex + 1] == 'n' // recognize '\n' only
                            && parseChar(lineBuffer[charIndex + 2]) == C_Apostrophe) {
                            for (int i = 0; i < 3; i++) {
                                tempStr += lineBuffer[charIndex];
                                charIndex++;
                            }
                            nextToken = { T_Character, lineIndex, tempStr};
                        }
                        else if (charIndex < lineBuffer.size()
                                 && parseChar(lineBuffer[charIndex]) != C_Apostrophe // not accept '''
                                 && parseChar(lineBuffer[charIndex]) != C_Backslash  // not accept '\'
                                 && charIndex + 1 < lineBuffer.size()
                                 && parseChar(lineBuffer[charIndex + 1]) == C_Apostrophe) {
                            for (int i = 0; i < 2; i++) {
                                tempStr += lineBuffer[charIndex];
                                charIndex++;
                            }
                            nextToken = { T_Character, lineIndex, tempStr};
                        }
                        else {
                            nextToken = { T_Unknown, lineIndex, tempStr};
                        }
                        return;
                        case C_ExclamationMark:
                        charIndex++;
                        if (charIndex < lineBuffer.size()
                            && lineBuffer[charIndex] == '=') {
                            charIndex++;
                            nextToken = { T_Symbols, lineIndex, "!="};
                        }
                        else {
                            nextToken = { T_Unknown, lineIndex, "!"};
                        }
                        return;
                        case C_Symbol:
                        tempStr += lineBuffer[charIndex];
                        charIndex++;
                        if (charIndex < lineBuffer.size()
                            && parseChar(lineBuffer[charIndex]) == C_Symbol
                            && twoCharSymbols.find(tempStr + lineBuffer[charIndex]) != twoCharSymbols.end()) {
                            tempStr += lineBuffer[charIndex];
                            charIndex++;
                        }
                        nextToken = { T_Symbols, lineIndex, tempStr};
                        return;
                        case C_Punctuation:
                        tempStr += lineBuffer[charIndex];
                        charIndex++;
                        nextToken = { T_Punctuation, lineIndex, tempStr};
                        return;
                        case C_Slash:
                        if (charIndex + 1 < lineBuffer.size() && parseChar(lineBuffer[charIndex + 1]) == C_Slash) {
                            endLineFlag = true;
                        }
                        else {
                            tempStr += lineBuffer[charIndex];
                            charIndex++;
                            nextToken = { T_Symbols, lineIndex, tempStr};
                            return;
                        }
                        break;
                        default:
                        tempStr += lineBuffer[charIndex];
                        charIndex++;
                        nextToken = { T_Unknown, lineIndex, tempStr};
                        return;
                    }
                }
            }
        }
    }
    
    std::string tokenName(TokenType type) {
        switch (type) {
            case T_Number:
            return "NUMBER         ";
            case T_Character:
            return "CHARACTER      ";
            case T_Identifier:
            return "IDENTIFIER     ";
            case T_Punctuation:
            return "PUNCTUATION    ";
            case T_Keywords:
            return "KEYWORDS       ";
            case T_Symbols:
            return "SYMBOLS        ";
            case T_Unknown:
            return "UNKNOWN        ";
            case T_EOF:
            return "EOF            ";
            default:
            return "";
        }
    }
    
    void printCurrentToken() {
        std::cout << currentToken.lineNumber << " : \t" << tokenName(currentToken.type) << currentToken.lexeme << std::endl;
    }
    
    void printNextToken() {
        std::cout << nextToken.lineNumber << " : \t" << tokenName(nextToken.type) << nextToken.lexeme << std::endl;
    }
    
    Token getToken() {
        return currentToken;
    }
    
    Token peekToken() {
        return nextToken;
    }
};

#endif /* scanner_h */
