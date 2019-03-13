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

public:
    SymTable() {
        nullID = insert("g", "Z", "null", "zlit", "null", "", "", "public", 0);
        trueID = insert("g", "Z", "true", "zlit", "bool", "", "", "public", 0);
        falseID = insert("g", "Z", "false", "zlit", "bool", "", "", "public", 0);
        charSizeID = insert("g", "N", "1", "ilit", "int", "", "", "public", 0);
        intSizeID = insert("g", "N", "4", "ilit", "int", "", "", "public", 0);
        mainID = insert("g.main", "F", "main", "main", "", "void", "[]", "public", 0);

        iCode(0, FRAME, getSymID(mainID), getSymID(nullID), "", "");
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
    
    void generateTCode() {
        std::ofstream targetFile;
        targetFile.open ("out.asm", std::ios::out | std::ios::trunc);
        for (int i = 0; i < quad.size(); i++) {
            targetFile << "  " << quad[i].lineNumber << ":\t";
            if (quad[i].label.size() == 0)
                targetFile << "        \t";
            else
                targetFile << quad[i].label << "\t";
            targetFile << getICodeOpStr(quad[i].opcode) << "\t";
            targetFile << quad[i].operand1 << "\t";
            targetFile << quad[i].operand2 << "\t";
            targetFile << quad[i].operand3 << "\t";
            targetFile << "\n";
        }
        targetFile.close();
    }
};

#endif /* SymTable_h */
