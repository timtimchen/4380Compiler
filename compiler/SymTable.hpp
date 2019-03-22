//
//  SymTable.hpp
//  compiler
//
//  Created by jing hong chen on 1/26/19.
//  Copyright © 2019 jing hong chen. All rights reserved.
//

#ifndef SymTable_h
#define SymTable_h
#define SYMID_START 100

#include <iostream>
#include <fstream>
#include <string>
#include <map>

enum ICODEOP {
    ADD,
    ADI,
    SUB,
    MUL,
    DIV,
    LT,
    GT,
    NE,
    EQ,
    LE,
    GE,
    AND,
    OR,
    BF,
    BT,
    JMP,
    PUSH,
    POP,
    PEEK,
    FRAME,
    CALL,
    RTN,
    RETURN,
    FUNC,
    NEWI,
    NEW,
    MOV,
    MOVI,
    WRITE,
    READ,
    WRTC,
    WRTI,
    RDC,
    RDI,
    REF,
    AEF,
    STOP,
    NOP
};

struct QUAD {
    int lineNumber;
    ICODEOP opcode;
    std::string operand1;
    std::string operand2;
    std::string operand3;
    std::string label;
};

class SymTable {
private:
    int nextID = SYMID_START;
    int nullID, trueID, falseID, charSizeID, intSizeID, mainID;
    std::map<int, std::string> symScope;
    std::map<int, std::string> symID;
    std::map<int, std::string> symValue;
    std::map<int, std::string> symKind;
    std::map<int, std::string> symType;
    std::map<int, std::string> symReturnType;
    std::map<int, std::string> symParam;
    std::map<int, std::string> symAccessMod;
    std::map<int, int> symOffset;
    // for icode generator
    int iCodeCounter = 0;
    int labelCounter = 0;
    std::vector<QUAD> quad;
    std::vector<QUAD> sQuad;
    bool isStaticInitICode = false;
    std::vector<std::string> tCode;

public:
    SymTable() {
        nullID = insert("g", "Z", "null", "zlit", "null", "", "", "public", 0);
        trueID = insert("g", "Z", "true", "zlit", "bool", "", "", "public", 0);
        falseID = insert("g", "Z", "false", "zlit", "bool", "", "", "public", 0);
        charSizeID = insert("g", "N", "1", "ilit", "int", "", "", "public", 0);
        intSizeID = insert("g", "N", "4", "ilit", "int", "", "", "public", 0);
        mainID = insert("g.main", "F", "main", "main", "", "void", "[]", "public", 0);

        iCode(0, FRAME, getSymID(mainID), "NULL", "", "");
        iCode(0, CALL, getSymID(mainID), "", "", "");
        iCode(0, STOP, "", "", "", "");
    }
    
    int insert(std::string scope,
                std::string idType,
                std::string value,
                std::string kind,
                std::string type,
                std::string returnType,
                std::string parameter,
                std::string accessMod,
               int offset) {
        std::string symid = idType + std::to_string(nextID);
        symScope.insert(std::pair<int, std::string>(nextID, scope));
        symID.insert(std::pair<int, std::string>(nextID, symid));
        symValue.insert(std::pair<int, std::string>(nextID, value));
        symKind.insert(std::pair<int, std::string>(nextID, kind));
        symType.insert(std::pair<int, std::string>(nextID, type));
        symReturnType.insert(std::pair<int, std::string>(nextID, returnType));
        symParam.insert(std::pair<int, std::string>(nextID, parameter));
        symAccessMod.insert(std::pair<int, std::string>(nextID, accessMod));
        symOffset.insert(std::pair<int, int>(nextID, offset));

        nextID++;
        return nextID - 1;  //return the number id of the new symbol record
    }
    
    void updateOffset(int id, int newOffset) {
        symOffset[id] = newOffset;
    }
    
    void updateParam(int id, std::string newParam) {
        symParam[id] = newParam;
    }
    
    void updateName(int id) {
        symValue[id] = symID[id];
    }
    
    int searchValue(std::string scope, std::string value) {
        for (auto it = symValue.begin(); it != symValue.end(); it++) {
            if (it->second == value) {
                if (symScope[it->first] == scope) {
                    return it->first;
                }
            }
        }
        return 0; //return zero if not found
    }
    
    int getClassIDFromObject(int id) {
        int tempId = searchValue("g", getType(id));
        if (tempId != 0 && getKind(tempId) == "Class")
            return tempId;
        else
            return 0;
    }
    
    std::string getScope(int id) {
        return symScope[id];
    }
    
    std::string getSymID(int id) {
        return symID[id];
    }
    
    std::string getValue(int id) {
        return symValue[id];
    }
    
    std::string getKind(int id) {
        return symKind[id];
    }
    
    std::string getType(int id) {
        return symType[id];
    }
    
    std::string getReturnType(int id) {
        return symReturnType[id];
    }
    
    std::string getParam(int id) {
        return symParam[id];
    }
    
    std::string getAccessMod(int id) {
        return symAccessMod[id];
    }
    
    int getOffset(int id) {
        return symOffset[id];
    }
    
    void print(int id) {
        std::cout << "Scope:\t\t" << getScope(id) << std::endl;
        std::cout << "Symid:\t\t" << getSymID(id) << std::endl;
        std::cout << "Value:\t\t" << getValue(id) << std::endl;
        std::cout << "Kind:\t\t" << getKind(id) << std::endl;
        std::cout << "Data:";
        if (getType(id).length() > 0) {
            std::cout << "\n\t\t\ttype: " << getType(id);
        }
        if (getReturnType(id).length() > 0) {
            std::cout << "\n\t\t\truturnType: " << getReturnType(id);
        }
        if (getParam(id).length() > 0) {
            std::cout << "\n\t\t\tParam: " << getParam(id);
        }
        if (getAccessMod(id).length() > 0) {
            std::cout << "\n\t\t\taccessMod: " << getAccessMod(id);
        }
        if (getOffset(id) > 0) {
            std::cout << "\n\t\t\tOffset: " << getOffset(id);
        }
        std::cout << "\n";
    }
    
    void printAll() {
        for (int i = SYMID_START; i < nextID; i++) {
            print(i);
            std::cout << std::endl;
        }
    }
    
    // icode generator
    void iCode(int line, ICODEOP op, std::string opr1, std::string opr2, std::string opr3, std::string lbl) {
        if (isStaticInitICode) {
            QUAD newQuad = {line, op, opr1, opr2, opr3, lbl};
            sQuad.push_back(newQuad);
        }
        else {
            iCodeCounter++;
            if (iCodeCounter > quad.size()) {
                QUAD newQuad = {line, op, opr1, opr2, opr3, lbl};
                quad.push_back(newQuad);
            }
            else {
                quad[quad.size() - 1].lineNumber = line;
                quad[quad.size() - 1].opcode = op;
                quad[quad.size() - 1].operand1 = opr1;
                quad[quad.size() - 1].operand2 = opr2;
                quad[quad.size() - 1].operand3 = opr3;
                if (lbl.size() > 0) {
                    backPatching(quad[quad.size() - 1].label, lbl);
                    quad[quad.size() - 1].label = lbl;
                }
            }
        }
    }
    
    void iCodeLabel(std::string lbl) {
        if (iCodeCounter + 1 > quad.size()) {
            QUAD newQuad = {0, NOP, "", "", "", lbl};
            quad.push_back(newQuad);
        }
        else {
            backPatching(quad[quad.size() - 1].label, lbl);
            quad[quad.size() - 1].label = lbl;
        }
    }
    
    void setStaticInitICode(bool value) {
        isStaticInitICode = value;
    }
    
    void dumpStaticICode() {
        iCodeCounter += sQuad.size();
        for (int i = 0; i < sQuad.size(); i++)
            quad.push_back(sQuad[i]);
        sQuad.clear();
    }
    
    void backPatching(std::string findLabel, std::string replaceLabel) {
        for (int i = 0; i < quad.size(); i++) {
            if (quad[i].opcode == JMP && quad[i].operand1 == findLabel)
                quad[i].operand1 = replaceLabel;
            else if (quad[i].opcode == BF && quad[i].operand2 == findLabel)
                quad[i].operand2 = replaceLabel;
            else if (quad[i].opcode == BT && quad[i].operand2 == findLabel)
                quad[i].operand2 = replaceLabel;
        }
    }
    
    int getNewLabelCount() {
        labelCounter++;
        return labelCounter;
    }
    
    std::string getICodeOpStr(ICODEOP iCodeOp) {
        switch (iCodeOp) {
            case ADD:
                return "ADD   ";
                break;
            case ADI:
                return "ADI   ";
                break;
            case SUB:
                return "SUB   ";
                break;
            case MUL:
                return "MUL   ";
                break;
            case DIV:
                return "DIV   ";
                break;
            case LT:
                return "LT    ";
                break;
            case GT:
                return "GT    ";
                break;
            case NE:
                return "NE    ";
                break;
            case EQ:
                return "EQ    ";
                break;
            case LE:
                return "LE    ";
                break;
            case GE:
                return "GE    ";
                break;
            case AND:
                return "AND   ";
                break;
            case OR:
                return "OR    ";
                break;
            case BF:
                return "BF    ";
                break;
            case BT:
                return "BT    ";
                break;
            case JMP:
                return "JMP   ";
                break;
            case PUSH:
                return "PUSH  ";
                break;
            case POP:
                return "POP   ";
                break;
            case PEEK:
                return "PEEK  ";
                break;
            case FRAME:
                return "FRAME ";
                break;
            case CALL:
                return "CALL  ";
                break;
            case RTN:
                return "RTN   ";
                break;
            case RETURN:
                return "RETURN";
                break;
            case FUNC:
                return "FUNC  ";
                break;
            case NEWI:
                return "NEWI  ";
                break;
            case NEW:
                return "NEW   ";
                break;
            case MOV:
                return "MOV   ";
                break;
            case MOVI:
                return "MOVI  ";
                break;
            case WRITE:
                return "WRITE ";
                break;
            case READ:
                return "READ  ";
                break;
            case WRTC:
                return "WRTC  ";
                break;
            case WRTI:
                return "WRTI  ";
                break;
            case RDC:
                return "RDC   ";
                break;
            case RDI:
                return "RDI   ";
                break;
            case REF:
                return "REF   ";
                break;
            case AEF:
                return "AEF   ";
                break;
            case STOP:
                return "STOP  ";
                break;
            default:
                return "";
                break;
        }
    }
    
    std::string printICode(QUAD code) {
        std::string result;
        result = "  " + std::to_string(code.lineNumber) + ":\t";
        if (code.label.size() == 0)
            result += "        \t";
        else
            result += code.label + "\t";
        result += getICodeOpStr(code.opcode) + "\t";
        result += code.operand1 + "\t";
        result += code.operand2 + "\t";
        result += code.operand3 + "\t";
        return result;
    }
    
    void printAllICode() {
        for (int i = 0; i < quad.size(); i++)
            std::cout << printICode(quad[i]) << std::endl;
    }
    
    int calculateParamSize(std::string param) {
        int paramNum = 0;
        if (param.length() > 2) {
            paramNum = 1;
            for (int i = 0; i < param.length(); i++) {
                if (param[i] == ',') paramNum++;
            }
        }
        return paramNum * 4;
    }
    
    void loadDataCode(std::string symIdStr, std::string regName) {
        int symId = std::stoi(symIdStr.substr(1));
        if (getKind(symId) == "ilit") {
            tCode.push_back("\t\t\t\tLDR\t\t" + regName + ", " + symIdStr);
            return;
        }
        else if (getKind(symId) == "clit") {
            tCode.push_back("\t\t\t\tLDB\t\t" + regName + ", " + symIdStr);
            return;
        }
        else if (getKind(symId) == "ivar") {
            tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
            tCode.push_back("\t\t\t\tADI\t\tR0, -8");
            tCode.push_back("\t\t\t\tLDR\t\t" + regName + ", R0");
            tCode.push_back("\t\t\t\tMOV\t\tR0, " + regName);
            tCode.push_back("\t\t\t\tADI\t\tR0, " + std::to_string(getOffset(symId)));
        }
        else if (symIdStr[0] == 'R') {
            tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
            tCode.push_back("\t\t\t\tADI\t\tR0, -" + std::to_string(getOffset(symId)));
            tCode.push_back("\t\t\t\tLDR\t\t" + regName + ", R0");
            tCode.push_back("\t\t\t\tMOV\t\tR0, " + regName);
        }
        else {
            tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
            tCode.push_back("\t\t\t\tADI\t\tR0, -" + std::to_string(getOffset(symId)));
        }
        if (getType(symId) == "char")
            tCode.push_back("\t\t\t\tLDB\t\t" + regName + ", R0");
        else
            tCode.push_back("\t\t\t\tLDR\t\t" + regName + ", R0");
    }
    
    void storeDataCode(std::string symIdStr, std::string regName) {
        int symId = std::stoi(symIdStr.substr(1));
        if (getKind(symId) == "ivar") {
            tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
            tCode.push_back("\t\t\t\tADI\t\tR0, -8");
            tCode.push_back("\t\t\t\tLDR\t\tR4, R0");
            tCode.push_back("\t\t\t\tMOV\t\tR0, R4");
            tCode.push_back("\t\t\t\tADI\t\tR0, " + std::to_string(getOffset(symId)));
        }
        else if (symIdStr[0] == 'R') {
            tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
            tCode.push_back("\t\t\t\tADI\t\tR0, -" + std::to_string(getOffset(symId)));
            tCode.push_back("\t\t\t\tLDR\t\tR4, R0");
            tCode.push_back("\t\t\t\tMOV\t\tR0, R4");
        }
        else {
            tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
            tCode.push_back("\t\t\t\tADI\t\tR0, -" + std::to_string(getOffset(symId)));
        }
        if (getType(symId) == "char")
            tCode.push_back("\t\t\t\tSTB\t\t" + regName + ", R0");
        else
            tCode.push_back("\t\t\t\tSTR\t\t" + regName + ", R0");
    }
    
    void getLocationCode(std::string symIdStr, std::string regName) {
        int symId = std::stoi(symIdStr.substr(1));
        if (getKind(symId) == "ivar") {
            tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
            tCode.push_back("\t\t\t\tADI\t\tR0, -8");
            tCode.push_back("\t\t\t\tLDR\t\t" + regName + ", R0");
            tCode.push_back("\t\t\t\tMOV\t\tR0, " + regName);
            tCode.push_back("\t\t\t\tADI\t\tR0, " + std::to_string(getOffset(symId)));
            tCode.push_back("\t\t\t\tMOV\t\t" + regName + ", R0");
        }
        else if (symIdStr[0] == 'R') {
            tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
            tCode.push_back("\t\t\t\tADI\t\tR0, -" + std::to_string(getOffset(symId)));
            tCode.push_back("\t\t\t\tLDR\t\t" + regName + ", R0");
        }
        else {
            tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
            tCode.push_back("\t\t\t\tADI\t\tR0, -" + std::to_string(getOffset(symId)));
            tCode.push_back("\t\t\t\tMOV\t\t" + regName + ", R0");
        }
    }
    
    int getASCIIcode(std::string charLit) {
        if (charLit == "\'\\n\'") {
            return 10;
        }
        return charLit[1];
    }
    
    void generateTCode() {
        // generate global data
        tCode.push_back("OverF\t\t.INT\t-999999");
        tCode.push_back("UnderF\t\t.INT\t-111111");
        tCode.push_back("newline\t\t.BYT\t10");
        tCode.push_back("space\t\t.BYT\t32");

        for (int i = SYMID_START; i < nextID; i++) {
            if (getKind(i) == "ilit") {
                tCode.push_back(getSymID(i) + "\t\t.INT\t" + getValue(i));
            }
            else if (getKind(i) == "clit") {
                tCode.push_back(getSymID(i) + "\t\t.BYT\t" + std::to_string(getASCIIcode(getValue(i))));
            }
        }

        for (int i = 0; i < quad.size(); i++) {
            switch (quad[i].opcode) {
                case ADD:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case ADI:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case SUB:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case MUL:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case DIV:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case LT:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case GT:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case NE:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case EQ:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case LE:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case GE:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case AND:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case OR:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case BF:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case BT:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case JMP:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case PUSH:
                {
                    tCode.push_back("\t\t\t\tMOV\t\tR7, FP");
                    tCode.push_back("\t\t\t\tADI\t\tFP, -4");
                    tCode.push_back("\t\t\t\tLDR\t\tR6, FP");
                    tCode.push_back("\t\t\t\tMOV\t\tFP, R6");
                    tCode.push_back(";" + printICode(quad[i]));
                    loadDataCode(quad[i].operand1, "R1");
                    tCode.push_back("\t\t\t\tMOV\t\tFP, R7");
                    tCode.push_back("\t\t\t\tSTR\t\tR1, SP");
                    tCode.push_back("\t\t\t\tADI\t\tSP, -4");
                }
                    break;
                case POP:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case PEEK:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    tCode.push_back("\t\t\t\tLDR\t\tR6, SP");
                    storeDataCode(quad[i].operand1, "R6");
                }
                    break;
                case FRAME:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    int funcId = std::stoi(quad[i].operand1.substr(1));
                    int paramSize = 12 + calculateParamSize(getParam(funcId));
                    // Set 'this' pointer
                    if (quad[i].operand2 == "NULL") {
                        // clear R6
                        tCode.push_back("\t\t\t\tSUB\t\tR6, R6");
                    }
                    else if (quad[i].operand2 == "this") {
                        // copy 'this' pointer to R6
                        tCode.push_back("\t\t\t\tMOV\t\tR7, FP");
                        tCode.push_back("\t\t\t\tADI\t\tR7, -8");
                        tCode.push_back("\t\t\t\tLDR\t\tR6, R7");
                    }
                    else {
                        loadDataCode(quad[i].operand2, "R6");
                    }
                    // Test for overflow
                    tCode.push_back("\t\t\t\tMOV\t\tR5, SP");
                    tCode.push_back("\t\t\t\tADI\t\tR5, -" + std::to_string(paramSize));
                    tCode.push_back("\t\t\t\tCMP\t\tR5, SL");
                    tCode.push_back("\t\t\t\tBLT\t\tR5, OVERFLOW");
                    // Save off current FP in a Register
                    tCode.push_back("\t\t\t\tMOV\t\tR3, FP");
                    // Point at Current Activation Record (FP = SP)
                    tCode.push_back("\t\t\t\tMOV\t\tFP, SP");
                    // Adjust Stack Pointer for Return Address
                    tCode.push_back("\t\t\t\tADI\t\tSP, -4");
                    // Store PFP to Top of Stack
                    tCode.push_back("\t\t\t\tSTR\t\tR3, SP");
                    // Adjust Stack Pointer for PFP
                    tCode.push_back("\t\t\t\tADI\t\tSP, -4");
                    // Store this pointer to Top of Stack
                    tCode.push_back("\t\t\t\tSTR\t\tR6, SP");
                    // Adjust Stack Pointer for this
                    tCode.push_back("\t\t\t\tADI\t\tSP, -4");
                }
                    break;
                case CALL:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    tCode.push_back("\t\t\t\tMOV\t\tR1, PC");
                    tCode.push_back("\t\t\t\tADI\t\tR1, 36");
                    tCode.push_back("\t\t\t\tSTR\t\tR1, FP");
                    tCode.push_back("\t\t\t\tJMP\t\t" + quad[i].operand1);
                }
                    break;
                case RTN:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    // De-allocate Current Activation Record
                    tCode.push_back("\t\t\t\tMOV\t\tSP, FP");
                    // Test for Underflow (SP > SB)
                    tCode.push_back("\t\t\t\tMOV\t\tR5, FP");
                    tCode.push_back("\t\t\t\tCMP\t\tR5, SB");
                    tCode.push_back("\t\t\t\tBGT\t\tR5, UNDERFLOW");
                    // Load Return Address from the Frame
                    tCode.push_back("\t\t\t\tLDR\t\tR5, FP");
                    // Load PFP from the Frame
                    tCode.push_back("\t\t\t\tMOV\t\tR6, FP");
                    tCode.push_back("\t\t\t\tADI\t\tR6, -4");
                    tCode.push_back("\t\t\t\tLDR\t\tFP, R6");
                    // Jump using JMR to Return Address
                    tCode.push_back("\t\t\t\tJMR\t\tR5");
                }
                    break;
                case RETURN:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    // De-allocate Current Activation Record
                    tCode.push_back("\t\t\t\tMOV\t\tSP, FP");
                    // Test for Underflow (SP > SB)
                    tCode.push_back("\t\t\t\tMOV\t\tR5, FP");
                    tCode.push_back("\t\t\t\tCMP\t\tR5, SB");
                    tCode.push_back("\t\t\t\tBGT\t\tR5, UNDERFLOW");
                    // Load Return Address from the Frame
                    tCode.push_back("\t\t\t\tLDR\t\tR5, FP");
                    // Store Return Value on Top of Stack
                    if (quad[i].operand1 == "this") {
                        // copy 'this' pointer to the return area
                        tCode.push_back("\t\t\t\tMOV\t\tR6, FP");
                        tCode.push_back("\t\t\t\tADI\t\tR6, -8");
                        tCode.push_back("\t\t\t\tLDR\t\tR7, R6");
                        tCode.push_back("\t\t\t\tADI\t\tR6, 8");
                        tCode.push_back("\t\t\t\tSTR\t\tR7, R6");
                    }
                    else {
                        loadDataCode(quad[i].operand1, "R7");
                        tCode.push_back("\t\t\t\tSTR\t\tR7, FP");
                   }
                    // Load PFP from the Frame
                    tCode.push_back("\t\t\t\tMOV\t\tR6, FP");
                    tCode.push_back("\t\t\t\tADI\t\tR6, -4");
                    tCode.push_back("\t\t\t\tLDR\t\tFP, R6");
                    // Jump using JMR to Return Address
                    tCode.push_back("\t\t\t\tJMR\t\tR5");
                }
                    break;
                case FUNC:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    int funcId = std::stoi(quad[i].operand1.substr(1));
                    int funcBodySize = getOffset(funcId) - 12 - calculateParamSize(getParam(funcId));
                    // Test for overflow
                    tCode.push_back(quad[i].operand1 + "\t\tMOV\t\tR5, SP");
                    tCode.push_back("\t\t\t\tADI\t\tR5, -" + std::to_string(funcBodySize));
                    tCode.push_back("\t\t\t\tMOV\t\tR6, R5");
                    tCode.push_back("\t\t\t\tCMP\t\tR5, SL");
                    tCode.push_back("\t\t\t\tBLT\t\tR5, OVERFLOW");
                    // Allocate space for Temporary and Local Variables
                    tCode.push_back("\t\t\t\tMOV\t\tSP, R6");
                }
                    break;
                case NEWI:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    tCode.push_back("\t\t\t\tMOV\t\tR5, SL");
                    tCode.push_back("\t\t\t\tADI\t\tR5, " + quad[i].operand1);
                    tCode.push_back("\t\t\t\tMOV\t\tR6, R5");
                    // Test for overflow
                    tCode.push_back("\t\t\t\tCMP\t\tR5, SP");
                    tCode.push_back("\t\t\t\tBGT\t\tR5, OVERFLOW");
                    // return the allocated address and renew SL
                    tCode.push_back("\t\t\t\tMOV\t\tR5, SL");
                    tCode.push_back("\t\t\t\tMOV\t\tSL, R6");
                    storeDataCode(quad[i].operand2, "R5");
                }
                    break;
                case NEW:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case MOV:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    loadDataCode(quad[i].operand1, "R1");
                    storeDataCode(quad[i].operand2, "R1");
                }
                    break;
                case MOVI:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case WRITE:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    loadDataCode(quad[i].operand1, "R3");
                    tCode.push_back("\t\t\t\tTRP\t\t1");
                }
                    break;
                case READ:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case WRTC:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    loadDataCode(quad[i].operand1, "R3");
                    tCode.push_back("\t\t\t\tTRP\t\t3");
                }
                    break;
                case WRTI:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    loadDataCode(quad[i].operand1, "R3");
                    tCode.push_back("\t\t\t\tTRP\t\t1");
                }
                    break;
                case RDC:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case RDI:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case REF:
                {
                    int tId = std::stoi(quad[i].operand3.substr(1));
                    tCode.push_back(";" + printICode(quad[i]));
                    if (quad[i].operand1 == "this") {
                        tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
                        tCode.push_back("\t\t\t\tADI\t\tR0, -8");
                        tCode.push_back("\t\t\t\tLDR\t\tR1, R0");
                    }
                    else {
                        loadDataCode(quad[i].operand1, "R1");
                    }
                    tCode.push_back("\t\t\t\tADI\t\tR1, " + std::to_string(getOffset(std::stoi(quad[i].operand2.substr(1)))));
                    // store the address on R1 to a Reference variable
                    tCode.push_back("\t\t\t\tMOV\t\tR0, FP");
                    tCode.push_back("\t\t\t\tADI\t\tR0, -" + std::to_string(getOffset(tId)));
                    tCode.push_back("\t\t\t\tSTR\t\tR1, R0");
                }
                    break;
                case AEF:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    
                }
                    break;
                case STOP:
                {
                    tCode.push_back(";" + printICode(quad[i]));
                    tCode.push_back("\t\t\t\tLDB\t\tR3, newline");
                    tCode.push_back("\t\t\t\tTRP\t\t3");
                    tCode.push_back("\t\t\t\tTRP\t\t0");
                }
                    break;
                default:
                    tCode.push_back("\t\t\t\tNOP\t\t\t\t\t\t;SOME ERROR HAPPENED");
                    break;
            }
        }
        // generate overflow checking code
        tCode.push_back("OVERFLOW\t\tLDB\t\tR3, newline");
        tCode.push_back("\t\t\t\tTRP\t\t3");
        tCode.push_back("\t\t\t\tLDR\t\tR3, OverF");
        tCode.push_back("\t\t\t\tTRP\t\t1");
        tCode.push_back("\t\t\t\tLDB\t\tR3, newline");
        tCode.push_back("\t\t\t\tTRP\t\t3");
        tCode.push_back("\t\t\t\tTRP\t\t0");
        // generate underflow checking code
        tCode.push_back("UNDERFLOW\t\tLDB\t\tR3, newline");
        tCode.push_back("\t\t\t\tTRP\t\t3");
        tCode.push_back("\t\t\t\tLDR\t\tR3, OverF");
        tCode.push_back("\t\t\t\tTRP\t\t1");
        tCode.push_back("\t\t\t\tLDB\t\tR3, UnderF");
        tCode.push_back("\t\t\t\tTRP\t\t3");
        tCode.push_back("\t\t\t\tTRP\t\t0");

        saveTCodeTofile();
    }
    
    void saveTCodeTofile() {
        std::ofstream targetFile;
        targetFile.open ("tcode.asm", std::ios::out | std::ios::trunc);
        for (int i = 0; i < tCode.size(); i++) {
            targetFile << tCode[i];
            targetFile << "\n";
        }
        targetFile.close();
    }
};

#endif /* SymTable_h */
