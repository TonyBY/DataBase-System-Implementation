#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include "ParseTree.h"

namespace Util {
    
    static char* StatisticsFile = "Statistics.txt";

    std::string randomStr(const int len);
    
    bool isEmptyFile(std::ifstream &in_file);
    
    std::string ParseTreeToString(Operand *operand);
    std::string ParseTreeToString(ComparisonOp *comp_op);
    std::string ParseTreeToString(OrList *or_list);
    std::string ParseTreeToString(AndList *and_list);
    std::string ParseTreeToString(std::vector<AndList*> and_lists);
    
    // split an AndList into several single AndLists (one single AndList only contains one OrList, that is, rightAnd == NULL)
    std::vector<AndList*> splitAndList(AndList *and_list); 
    AndList* getParseTreeFromVector(std::vector<AndList*> vec);

    void removeRelNameFromPred(ComparisonOp* predicate);
    void removeRelNameFromPred(OrList* predicate);
    void removeRelNameFromPred(AndList* predicate);
    void removeRelNameFromPred(NameList* atts);
    void removeRelNameFromPred(FuncOperator* func);
    

    std::string getPrefix(std::string input, char separator);
    std::string getSuffix(std::string input, char separator);
    std::vector<std::string> splitString(std::string input, char separator);
    

    template<typename T>
    std::string toString(T& a) {
        std::stringstream ss;
        ss << a;
        return ss.str();
    } 

    template<typename T>
    T fromString(std::string& s) {
        T res;
        std::istringstream(s) >> res;
        return res; 
    } 

    template<typename T>
    int catArrays(T* merged, T* left, T* right, int leftLength, int rightLength) {
        for (int i = 0; i < leftLength; i++) {
            merged[i] = left[i];
        }
        for (int j = 0; j < rightLength; j++) {
            merged[leftLength+j] = right[j];
        }
        return leftLength + rightLength;
    }

    template<typename T>
    void printVector(std::vector<T> vec) {
        for (int i = 0; i < vec.size(); i++) {
            std::cout << Util::toString<T>(vec[i]);
            if (i < vec.size() - 1){
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }

}

#endif