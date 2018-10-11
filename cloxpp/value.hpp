//
//  value.hpp
//  cloxpp
//
//  Created by Ahmad Alhashemi on 2018-09-01.
//  Copyright © 2018 Ahmad Alhashemi. All rights reserved.
//

#ifndef value_hpp
#define value_hpp

#include "common.hpp"
#include <variant>

using Value = std::variant<double, bool, std::monostate, std::string>;

struct OutputVisitor {
    void operator()(const double d) const { std::cout << d; }
    void operator()(const bool b) const { std::cout << (b ? "true" : "false"); }
    void operator()(const std::monostate n) const { std::cout << "nil"; }
    void operator()(const std::string s) const { std::cout << s; }
};

inline std::ostream& operator<<(std::ostream& os, const Value& v) {
    std::visit(OutputVisitor(), v);
    return os;
}

struct FalsinessVisitor {
    bool operator()(const double d) const { return false; }
    bool operator()(const bool b) const { return !b; }
    bool operator()(const std::monostate n) const { return true; }
    bool operator()(const std::string& s) const { return false; }
};

inline bool isFalsy(const Value& v) {
    return std::visit(FalsinessVisitor(), v);
}

#endif /* value_hpp */
