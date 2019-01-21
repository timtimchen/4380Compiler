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

class Compiler {
private:
    std::string sourceCodeFilename;

public:
    Compiler(std::string filename) {
        sourceCodeFilename = filename;
    }
    
    void lexicalAnalysis(std::string fileName) {
        Scanner scanner(sourceCodeFilename);
        scanner.fetchTokens();
        while (scanner.peekToken().type != T_EOF) {
            scanner.fetchTokens();
            scanner.printCurrentToken();
        }
        scanner.printNextToken();
    }
    
    void run() {
        lexicalAnalysis(sourceCodeFilename);
    }
};

#endif /* Compiler_h */
