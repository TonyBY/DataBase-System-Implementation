#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <sstream>

namespace Util {
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
}

#endif