//
//  vm.hpp
//  compiler
//
//  Created by jing hong chen on 1/9/19.
//  Copyright © 2019 jing hong chen. All rights reserved.
//

#ifndef vm_hpp
#define vm_hpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iterator>

#define REG_SIZE 13  // total general regesters
#define MEM_SIZE 60000  // total bytes of memory
#define FIX_LENGTH 12  // fixed length of an instrution
#define INT_SIZE 4  // size of an integer
#define JMP 1
#define JMR 2
#define BNZ 3
#define BGT 4
#define BLT 5
#define BRZ 6
#define MOV 7
#define LDA 8
#define STR 9
#define LDR 10
#define STB 11
#define LDB 12
#define ADD 13
#define ADI 14
#define SUB 15
#define MUL 16
#define DIV 17
#define AND 18
#define OR 19
#define CMP 20
#define TRP 21
#define STRI 22
#define LDRI 23
#define STBI 24
#define LDBI 25
#define NOP 100
#define _INT -4
#define _BYT -1

struct Instruction {
    int OpCode;
    int Oprand1;
    int Oprand2;
};

class VM {
private:
    // VM register, 0 - 7: general Rigsiter, 8: PC, 9: SL, 10: SP, 11: FP, 12: SB
    int REG[REG_SIZE];  // VM registers
    char MEM[MEM_SIZE];  // VM memory
    int memoryUsedCount;
    std::map<std::string, int> OpCodeTable;  // Operator Codes map (including Directives
    std::map<std::string, int> SymbolTable;  // Operator Codes map (including Directives
    
public:
    VM() {
        REG[8] = 0;
        memoryUsedCount = 0;
        
        OpCodeTable.insert(std::pair<std::string, int>("JMP", JMP));
        OpCodeTable.insert(std::pair<std::string, int>("JMR", JMR));
        OpCodeTable.insert(std::pair<std::string, int>("BNZ", BNZ));
        OpCodeTable.insert(std::pair<std::string, int>("BGT", BGT));
        OpCodeTable.insert(std::pair<std::string, int>("BLT", BLT));
        OpCodeTable.insert(std::pair<std::string, int>("BRZ", BRZ));
        OpCodeTable.insert(std::pair<std::string, int>("MOV", MOV));
        OpCodeTable.insert(std::pair<std::string, int>("LDA", LDA));
        OpCodeTable.insert(std::pair<std::string, int>("STR", STR));
        OpCodeTable.insert(std::pair<std::string, int>("LDR", LDR));
        OpCodeTable.insert(std::pair<std::string, int>("STB", STB));
        OpCodeTable.insert(std::pair<std::string, int>("LDB", LDB));
        OpCodeTable.insert(std::pair<std::string, int>("ADD", ADD));
        OpCodeTable.insert(std::pair<std::string, int>("ADI", ADI));
        OpCodeTable.insert(std::pair<std::string, int>("SUB", SUB));
        OpCodeTable.insert(std::pair<std::string, int>("MUL", MUL));
        OpCodeTable.insert(std::pair<std::string, int>("DIV", DIV));
        OpCodeTable.insert(std::pair<std::string, int>("AND", AND));
        OpCodeTable.insert(std::pair<std::string, int>("OR", OR));
        OpCodeTable.insert(std::pair<std::string, int>("CMP", CMP));
        OpCodeTable.insert(std::pair<std::string, int>("TRP", TRP));
        OpCodeTable.insert(std::pair<std::string, int>("NOP", NOP));
        OpCodeTable.insert(std::pair<std::string, int>(".INT", _INT));
        OpCodeTable.insert(std::pair<std::string, int>(".BYT", _BYT));
    }
    
    void loadInstruction(int addr, int opcode, int oprand1, int oprand2) {
        Instruction * ip = reinterpret_cast<Instruction *>(& MEM[addr]);
        ip->OpCode = opcode;
        ip->Oprand1 = oprand1;
        ip->Oprand2 = oprand2;
    }
    
    Instruction * fetchInstruction(int addr) {
        if (addr + FIX_LENGTH > MEM_SIZE) {
            return nullptr;
        }
        return reinterpret_cast<Instruction *>(& MEM[addr]);
    }
    
    // set a JMP instruction to the beginning of Memory, jump to the real start point
    void setProgramStartPoint(int startPoint) {
        loadInstruction(0, JMP, startPoint, 0);
    }
    
    void setByte(int addr, char data) {
        MEM[addr] = data;
    }
    
    void setInt(int addr, int data) {
        int *p = reinterpret_cast<int *>(& MEM[addr]);
        *p = data;
    }
    
    int getInt(int addr) {
        int *p = reinterpret_cast<int *>(& MEM[addr]);
        return *p;
    }
    
    bool isNumber(std::string str) {
        if (str.size() < 1) return false;
        std::string numStr;
        // check minus sign
        if (str.at(0) == '-') {
            // if it is signed, ignore the minus sign
            numStr = str.substr(1);
        } else {
            numStr = str;
        }
        return (numStr.size() > 0) && (numStr.find_first_not_of("0123456789") == std::string::npos);
    }
    
    bool isRegsterName(std::string name) {
        return (name == "SP") || (name == "SP,")
        || (name == "FP") || (name == "FP,")
        || (name == "SB") || (name == "SB,")
        || (name == "SL") || (name == "SL,")
        || (name.size() == 3 && name.at(0) == 'R' && name.at(2) == ',' && name.at(1) <= '7' && name.at(1) >= '0')
        || (name.size() == 2 && name.at(0) == 'R' && name.at(1) <= '7' && name.at(1) >= '0');
    }
    
    int getRegisterId(std::string name) {
        if (name == "PC") return 8;
        if (name == "SL" || name == "SL,") return 9;
        if (name == "SP" || name == "SP,") return 10;
        if (name == "FP" || name == "FP,") return 11;
        if (name == "SB" || name == "SB,") return 12;
        if (name.size() == 3 && name.at(0) == 'R' && name.at(2) == ',' && name.at(1) <= '7' && name.at(1) >= '0')
            return name.at(1) - '0';
        if (name.size() == 2 && name.at(0) == 'R' && name.at(1) <= '7' && name.at(1) >= '0')
            return name.at(1) - '0';
        return 0;
    }
    
    bool assemblyPass1(std::string fileName) {
        std::ifstream inputFile(fileName);
        if (inputFile) {
            std::string line;
            int lineCounter = 0;
            int addrCounter = FIX_LENGTH;  //reserve first 12 bytes for start JMP
            int tokenCounter = 0;
            // check the input assembly file by Pass 1
            //ignore all the whitespace
            while (std::getline(inputFile, line)) {
                lineCounter++;
                unsigned long firstCharPos = line.find_first_not_of(" \t\n\r\v\f");
                // skip empty lines and comments (start with ';')
                if (firstCharPos != std::string::npos && line.at(firstCharPos) != ';') {
                    // split a line into tokens
                    std::istringstream iss(line);
                    std::vector<std::string> tokens((std::istream_iterator<std::string>(iss)),std::istream_iterator<std::string>());
                    if (tokens.size() < 1) {
                        std::cout << "Command line too short. (line: " << lineCounter << ")\n";
                        return false;
                    }
                    if (tokens.size() < 2) {
                        if (OpCodeTable.find(tokens[0]) == OpCodeTable.end()) {
                            if (SymbolTable.find(tokens[0]) != SymbolTable.end()) {
                                std::cout << "Repeated Label. (line: " << lineCounter << ")\n";
                                return false;
                            }
                            // treat a single label as a NOP command
                            SymbolTable.insert(std::pair<std::string, int>(tokens[0], addrCounter));
                            addrCounter += FIX_LENGTH;
                        } else {
                            std::cout << "Command line too short. (line: " << lineCounter << ")\n";
                            return false;
                        }
                    } else {
                        tokenCounter = 0;
                        if (OpCodeTable.find(tokens[tokenCounter]) == OpCodeTable.end()) {
                            // if first token is a Label, check if it is a new one, if yes, put it in the Symbol table
                            if (SymbolTable.find(tokens[tokenCounter]) != SymbolTable.end()) {
                                std::cout << "Repeated Label. (line: " << lineCounter << ")\n";
                                return false;
                            }
                            SymbolTable.insert(std::pair<std::string, int>(tokens[tokenCounter], addrCounter));
                            tokenCounter++;
                            if (OpCodeTable.find(tokens[tokenCounter]) == OpCodeTable.end()) {
                                std::cout << "Error format command line. (line: " << lineCounter << ")\n";
                                return false;
                            }
                        } // after this checking, we found a OpCode, and the tokenCounter is pointing to the OpCode
                        if (OpCodeTable[tokens[tokenCounter]] > 0) {
                            // it is an Instruction
                            addrCounter += FIX_LENGTH;
                        } else {
                            // it is a directive
                            addrCounter += -OpCodeTable[tokens[tokenCounter]];
                        }
                    }
                }
            }
            // pass first checking step
            return true;
        } else {
            std::cout << "Cannot open the file: " << fileName << std::endl;
            return false;
        }
    }
    
    bool assemblyPass2(std::string fileName) {
        std::ifstream inputFile(fileName);
        if (inputFile) {
            std::string line;
            int lineCounter = 0;
            int addrCounter = FIX_LENGTH;   //reserve first 12 bytes for start JMP
            int tokenCounter = 0;
            bool programStart = false;
            // check the input assembly file by Pass 2, if commands are correct, load them to Memory
            //ignore all the whitespace
            while (std::getline(inputFile, line)) {
                lineCounter++;
                unsigned long firstCharPos = line.find_first_not_of(" \t\n\r\v\f");
                // skip empty lines and comments (start with ';')
                if (firstCharPos != std::string::npos && line.at(firstCharPos) != ';') {
                    // split a line into tokens
                    std::istringstream iss(line);
                    std::vector<std::string> tokens((std::istream_iterator<std::string>(iss)),std::istream_iterator<std::string>());
                    if (tokens.size() < 1) {
                        std::cout << "Command line too short. (line: " << lineCounter << ")\n";
                        return false;
                    }
                    if (tokens.size() < 2) {
                        if (OpCodeTable.find(tokens[0]) == OpCodeTable.end()) {
                            if (SymbolTable.find(tokens[0]) == SymbolTable.end()) {
                                std::cout << "Unexpected error  . (line: " << lineCounter << ")\n";
                                return false;
                            }
                            // treat a single label as a NOP command
                            loadInstruction(addrCounter, NOP, 0, 0);
                            addrCounter += FIX_LENGTH;
                        } else {
                            std::cout << "Command line too short. (line: " << lineCounter << ")\n";
                            return false;
                        }
                    } else {
                        tokenCounter = 0;
                        if (OpCodeTable.find(tokens[tokenCounter]) == OpCodeTable.end()) {
                            tokenCounter++;
                            if (OpCodeTable.find(tokens[tokenCounter]) == OpCodeTable.end()) {
                                std::cout << "Error format command line. (line: " << lineCounter << ")\n";
                                return false;
                            }
                        } // after this checking, we found a OpCode, and the tokenCounter is pointing to the OpCode
                        if (OpCodeTable[tokens[tokenCounter]] > 0) {
                            // it is an Instruction
                            if (!programStart) {
                                setProgramStartPoint(addrCounter);
                                programStart = true;
                            }
                            switch (OpCodeTable[tokens[tokenCounter]]) {
                                case JMP:
                                    if (tokenCounter + 1 < tokens.size() && SymbolTable.find(tokens[tokenCounter + 1]) != SymbolTable.end()) {
                                        loadInstruction(addrCounter, JMP, SymbolTable[tokens[tokenCounter + 1]], 0);
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case JMR:
                                    if (tokenCounter + 1 < tokens.size() && isRegsterName(tokens[tokenCounter + 1])) {
                                        loadInstruction(addrCounter, JMR, getRegisterId(tokens[tokenCounter + 1]), 0);
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case BNZ:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && SymbolTable.find(tokens[tokenCounter + 2]) != SymbolTable.end()) {
                                        loadInstruction(addrCounter, BNZ, getRegisterId(tokens[tokenCounter + 1]), SymbolTable[tokens[tokenCounter + 2]]);
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case BGT:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && SymbolTable.find(tokens[tokenCounter + 2]) != SymbolTable.end()) {
                                        loadInstruction(addrCounter, BGT, getRegisterId(tokens[tokenCounter + 1]), SymbolTable[tokens[tokenCounter + 2]]);
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case BLT:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && SymbolTable.find(tokens[tokenCounter + 2]) != SymbolTable.end()) {
                                        loadInstruction(addrCounter, BLT, getRegisterId(tokens[tokenCounter + 1]), SymbolTable[tokens[tokenCounter + 2]]);
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case BRZ:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && SymbolTable.find(tokens[tokenCounter + 2]) != SymbolTable.end()) {
                                        loadInstruction(addrCounter, BRZ, getRegisterId(tokens[tokenCounter + 1]), SymbolTable[tokens[tokenCounter + 2]]);
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case MOV:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && (tokens[tokenCounter + 2] == "PC" || isRegsterName(tokens[tokenCounter + 2]))) {
                                        loadInstruction(addrCounter, MOV, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case LDA:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && SymbolTable.find(tokens[tokenCounter + 2]) != SymbolTable.end()) {
                                        loadInstruction(addrCounter, LDA, getRegisterId(tokens[tokenCounter + 1]), SymbolTable[tokens[tokenCounter + 2]]);
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case STR:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1])) {
                                        if (isRegsterName(tokens[tokenCounter + 2])) {
                                            loadInstruction(addrCounter, STRI, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                        } else if (SymbolTable.find(tokens[tokenCounter + 2]) != SymbolTable.end()) {
                                            loadInstruction(addrCounter, STR, getRegisterId(tokens[tokenCounter + 1]), SymbolTable[tokens[tokenCounter + 2]]);
                                        } else {
                                            std::cout << "Command Line Error1. (line: " << lineCounter << ")\n";
                                            return false;
                                        }
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case LDR:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1])) {
                                        if (isRegsterName(tokens[tokenCounter + 2])) {
                                            loadInstruction(addrCounter, LDRI, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                        } else if (SymbolTable.find(tokens[tokenCounter + 2]) != SymbolTable.end()) {
                                            loadInstruction(addrCounter, LDR, getRegisterId(tokens[tokenCounter + 1]), SymbolTable[tokens[tokenCounter + 2]]);
                                        } else {
                                            std::cout << "Command Line Error1. (line: " << lineCounter << ")\n";
                                            return false;
                                        }
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case STB:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1])) {
                                        if (isRegsterName(tokens[tokenCounter + 2])) {
                                            loadInstruction(addrCounter, STBI, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                        } else if (SymbolTable.find(tokens[tokenCounter + 2]) != SymbolTable.end()) {
                                            loadInstruction(addrCounter, STB, getRegisterId(tokens[tokenCounter + 1]), SymbolTable[tokens[tokenCounter + 2]]);
                                        } else {
                                            std::cout << "Command Line Error1. (line: " << lineCounter << ")\n";
                                            return false;
                                        }
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case LDB:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1])) {
                                        if (isRegsterName(tokens[tokenCounter + 2])) {
                                            loadInstruction(addrCounter, LDBI, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                        } else if (SymbolTable.find(tokens[tokenCounter + 2]) != SymbolTable.end()) {
                                            loadInstruction(addrCounter, LDB, getRegisterId(tokens[tokenCounter + 1]), SymbolTable[tokens[tokenCounter + 2]]);
                                        } else {
                                            std::cout << "Command Line Error1. (line: " << lineCounter << ")\n";
                                            return false;
                                        }
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case ADD:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && isRegsterName(tokens[tokenCounter + 2])) {
                                        loadInstruction(addrCounter, ADD, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case ADI:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && isNumber(tokens[tokenCounter + 2])) {
                                        loadInstruction(addrCounter, ADI, getRegisterId(tokens[tokenCounter + 1]), std::stoi(tokens[tokenCounter + 2]));
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case SUB:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && isRegsterName(tokens[tokenCounter + 2])) {
                                        loadInstruction(addrCounter, SUB, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case MUL:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && isRegsterName(tokens[tokenCounter + 2])) {
                                        loadInstruction(addrCounter, MUL, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case DIV:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && isRegsterName(tokens[tokenCounter + 2])) {
                                        loadInstruction(addrCounter, DIV, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case AND:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && isRegsterName(tokens[tokenCounter + 2])) {
                                        loadInstruction(addrCounter, AND, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case OR:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && isRegsterName(tokens[tokenCounter + 2])) {
                                        loadInstruction(addrCounter, OR, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case CMP:
                                    if (tokenCounter + 2 < tokens.size() && isRegsterName(tokens[tokenCounter + 1]) && isRegsterName(tokens[tokenCounter + 2])) {
                                        loadInstruction(addrCounter, CMP, getRegisterId(tokens[tokenCounter + 1]), getRegisterId(tokens[tokenCounter + 2]));
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case TRP:
                                    if (tokenCounter + 1 < tokens.size() && tokens[tokenCounter + 1].size() == 1 && tokens[tokenCounter + 1].at(0) <= '4' && tokens[tokenCounter + 1].at(0) >= '0') {
                                        loadInstruction(addrCounter, TRP, tokens[tokenCounter + 1].at(0) - '0', 0);
                                    } else {
                                        std::cout << "Command Line Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                    break;
                                case NOP:
                                    loadInstruction(addrCounter, NOP, 0, 0);
                                    break;
                            }
                            addrCounter += FIX_LENGTH;
                        } else {
                            // it is a directive
                            if (tokenCounter + 1 < tokens.size()) {
                                if (OpCodeTable[tokens[tokenCounter]] == _BYT) {  // handle character format like 'c'
                                    if (tokens[tokenCounter + 1].size() == 3 && tokens[tokenCounter + 1].at(0) == '\'' && tokens[tokenCounter + 1].at(2) == '\'') {
                                        setByte(addrCounter, tokens[tokenCounter + 1].at(1));
                                    } else { // handle number format like 10
                                        if (tokens[tokenCounter + 1].find_first_not_of("0123456789") == std::string::npos) {
                                            setByte(addrCounter, static_cast<char>(std::stoi(tokens[tokenCounter + 1])));
                                        } else {
                                            std::cout << ".BYT data Format Error. (line: " << lineCounter << ")\n";
                                            return false;
                                        }
                                    }
                                } else {  // directive of .INT
                                    if (isNumber(tokens[tokenCounter + 1])) {
                                        setInt(addrCounter, std::stoi(tokens[tokenCounter + 1]));
                                    } else {
                                        std::cout << ".INT data Format Error. (line: " << lineCounter << ")\n";
                                        return false;
                                    }
                                }
                            } else {
                                std::cout << "Command Line too short. (line: " << lineCounter << ")\n";
                                return false;
                            }
                            addrCounter += -OpCodeTable[tokens[tokenCounter]];
                        }
                    }
                }
            }
            // pass second checking step
            memoryUsedCount = addrCounter; // store total bytes used by all codes and data
            return true;
        } else {
            std::cout << "Cannot open the file: " << fileName << std::endl;
            return false;
        }
    }
    
    void run() {
        bool programStop = false;
        REG[8] = 0; // setting the PC register, start from MEM[0]
        REG[9] = memoryUsedCount; // setting the SL register nextll.;ju,,;.l,.l,m to the last used byte
        REG[12] = MEM_SIZE - 4; // setting the SB register to the last slot of Memory
        REG[10] = REG[12]; // setting the SP register
        REG[11] = REG[10]; // setting the FP register, first pointing to out of memory
        while (!programStop && REG[8] < MEM_SIZE) {
            Instruction * ip = fetchInstruction(REG[8]);
            if (ip == nullptr) {
                std::cout << "Out of Memory!" << std::endl;
                return;
            }
            switch (ip -> OpCode) {
                case JMP:
                    REG[8] = ip -> Oprand1;
                    break;
                case JMR:
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE) {
                        REG[8] = REG[ip->Oprand1];
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case BNZ:
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 <= MEM_SIZE - FIX_LENGTH) {
                        if (REG[ip->Oprand1] != 0) {
                            REG[8] = ip->Oprand2;
                        } else {
                            REG[8] += FIX_LENGTH;
                        }
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case BGT:
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 <= MEM_SIZE - FIX_LENGTH) {
                        if (REG[ip->Oprand1] > 0) {
                            REG[8] = ip->Oprand2;
                        } else {
                            REG[8] += FIX_LENGTH;
                        }
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case BLT:
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 <= MEM_SIZE - FIX_LENGTH) {
                        if (REG[ip->Oprand1] < 0) {
                            REG[8] = ip->Oprand2;
                        } else {
                            REG[8] += FIX_LENGTH;
                        }
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case BRZ:
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 <= MEM_SIZE - FIX_LENGTH) {
                        if (REG[ip->Oprand1] == 0) {
                            REG[8] = ip->Oprand2;
                        } else {
                            REG[8] += FIX_LENGTH;
                        }
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case MOV:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 >= 0 && ip->Oprand2 < REG_SIZE) {
                        REG[ip->Oprand1] = REG[ip->Oprand2];
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case LDA:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 <= MEM_SIZE - INT_SIZE) {
                        REG[ip->Oprand1] = ip->Oprand2;
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case STR:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 <= MEM_SIZE - INT_SIZE) {
                        int *p = reinterpret_cast<int *>(&MEM[ip->Oprand2]);
                        *p = REG[ip->Oprand1];
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case LDR:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 <= MEM_SIZE - INT_SIZE) {
                        int *p = reinterpret_cast<int *>(&MEM[ip->Oprand2]);
                        REG[ip->Oprand1] = *p;
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case STB:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 < MEM_SIZE) {
                        MEM[ip->Oprand2] = static_cast<char>(REG[ip->Oprand1]);
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case LDB:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 < MEM_SIZE) {
                        REG[ip->Oprand1] = 0; // clear the register
                        REG[ip->Oprand1] = static_cast<int>(MEM[ip->Oprand2]);
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case ADD:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 >= 0 && ip->Oprand2 < REG_SIZE) {
                        REG[ip->Oprand1] += REG[ip->Oprand2];
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case ADI:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2) {
                        REG[ip->Oprand1] += ip->Oprand2;
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case SUB:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 >= 0 && ip->Oprand2 < REG_SIZE) {
                        REG[ip->Oprand1] -= REG[ip->Oprand2];
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case MUL:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 >= 0 && ip->Oprand2 < REG_SIZE) {
                        REG[ip->Oprand1] *= REG[ip->Oprand2];
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case DIV:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 >= 0 && ip->Oprand2 < REG_SIZE) {
                        REG[ip->Oprand1] /= REG[ip->Oprand2];
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case AND:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 >= 0 && ip->Oprand2 < REG_SIZE) {
                        if (REG[ip->Oprand1] == 0 || REG[ip->Oprand2] == 0) {
                            REG[ip->Oprand1] = 0;
                        } else {
                            REG[ip->Oprand1] = 1;
                        }
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case OR:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 >= 0 && ip->Oprand2 < REG_SIZE) {
                        if (REG[ip->Oprand1] == 0 && REG[ip->Oprand2] == 0) {
                            REG[ip->Oprand1] = 0;
                        } else {
                            REG[ip->Oprand1] = 1;
                        }
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case CMP:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 >= 0 && ip->Oprand2 < REG_SIZE) {
                        REG[ip->Oprand1] -= REG[ip->Oprand2];
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case TRP:
                    REG[8] += FIX_LENGTH;
                    switch (ip -> Oprand1) {
                        case 0:
                            programStop = true;
                            break;
                        case 1:
                            std::cout << REG[3];
                            break;
                        case 2:
                            std::cin >> REG[3];
                            break;
                        case 3:
                            std::cout << static_cast<char>(REG[3]);
                            break;
                        case 4:
                            REG[3] = getchar();
                            break;
                        default:
                            std::cout << "Unexpected Error!" << std::endl;
                            return;
                    }
                    break;
                case STRI:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 <= MEM_SIZE - INT_SIZE) {
                        int *p = reinterpret_cast<int *>(&MEM[REG[ip->Oprand2]]);
                        *p = REG[ip->Oprand1];
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case LDRI:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 <= MEM_SIZE - INT_SIZE) {
                        int *p = reinterpret_cast<int *>(&MEM[REG[ip->Oprand2]]);
                        REG[ip->Oprand1] = *p;
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case STBI:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 < MEM_SIZE) {
                        MEM[REG[ip->Oprand2]] = static_cast<char>(REG[ip->Oprand1]);
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case LDBI:
                    REG[8] += FIX_LENGTH;
                    if (ip->Oprand1 >= 0 && ip->Oprand1 < REG_SIZE && ip->Oprand2 < MEM_SIZE) {
                        REG[ip->Oprand1] = 0; // clear the register
                        REG[ip->Oprand1] = static_cast<int>(MEM[REG[ip->Oprand2]]);
                    } else {
                        std::cout << "Unexpected Error!" << std::endl;
                        return;
                    }
                    break;
                case NOP:
                    REG[8] += FIX_LENGTH;
                    break;
                default:
                    std::cout << "Unexpected OpCode Error!" << std::endl;
                    return;
            }
        }
    }
    
};

#endif /* vm_hpp */
