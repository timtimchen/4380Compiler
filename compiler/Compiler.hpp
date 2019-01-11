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
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <iterator>

#define DEBUGMODE true

enum TokenType {
    NUMBER,
    CHARACTER,
    IDENTIFIER,
    PUNCTUATION,
    KEYWORDS,
    SYMBOLS,
    UNKNOWN,
    EoF
};

struct Token {
    TokenType type;
    int lineNumber;
    std::string lexeme;
};

class Compiler {
private:
    std::map<std::string, int> keywordTable;

public:
    Compiler() {
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
    }
    
    bool lexicalAnalysis(std::string fileName) {
        std::ifstream inputFile(fileName);
        if (inputFile) {
            std::string line;
            int lineCounter = 0;
            while (std::getline(inputFile, line)) {
                lineCounter++;
            }
            return true;
        } else {
            std::cout << "Cannot open the file: " << fileName << std::endl;
            return false;
        }
    }
        
};

#endif /* Compiler_h */
