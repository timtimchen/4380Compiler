//
//  main.cpp
//  compiler
//
//  Created by jing hong chen on 1/9/19.
//  Copyright © 2019 jing hong chen. All rights reserved.
//

#include <iostream>
#include "Compiler.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        cout << "Please input the KXI source file name in the command line." << endl;
    }
    else {
        Compiler newCompiler = Compiler(argv[1]);
        newCompiler.run();
    }
    
    return 0;
}

