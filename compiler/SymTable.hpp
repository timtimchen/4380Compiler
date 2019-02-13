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
#include <string>
#include <map>

class SymTable {
private:
    int nextID = SYMID_START;
    std::map<int, std::string> symScope;
    std::map<int, std::string> symID;
    std::map<int, std::string> symValue;
    std::map<int, std::string> symKind;
    std::map<int, std::string> symType;
    std::map<int, std::string> symReturnType;
    std::map<int, std::string> symParam;
    std::map<int, std::string> symAccessMod;
public:
    int insert(std::string scope,
                std::string idType,
                std::string value,
                std::string kind,
                std::string type,
                std::string returnType,
                std::string parameter,
                std::string accessMod) {
        std::string symid = idType + std::to_string(nextID);
        symScope.insert(std::pair<int, std::string>(nextID, scope));
        symID.insert(std::pair<int, std::string>(nextID, symid));
        symValue.insert(std::pair<int, std::string>(nextID, value));
        symKind.insert(std::pair<int, std::string>(nextID, kind));
        symType.insert(std::pair<int, std::string>(nextID, type));
        symReturnType.insert(std::pair<int, std::string>(nextID, returnType));
        symParam.insert(std::pair<int, std::string>(nextID, parameter));
        symAccessMod.insert(std::pair<int, std::string>(nextID, accessMod));
        
        nextID++;
        return nextID - 1;  //return the number id of the new symbol record
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
        std::cout << "\n";
    }
    
    void printAll() {
        for (int i = SYMID_START; i < nextID; i++) {
            print(i);
            std::cout << std::endl;
        }
    }
};

#endif /* SymTable_h */
