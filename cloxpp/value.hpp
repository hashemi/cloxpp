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
#include "opcode.hpp"
#include <variant>
#include <memory>

class Chunk;

class Function {
private:
    int arity;
    std::string name;
    std::shared_ptr<Chunk> chunk;

public:
    Function(int arity, const std::string& name):
        arity(arity), name(name), chunk(std::make_shared<Chunk>()) {}
    
    const std::string& getName() const { return name; }

    bool operator==(const Function& rhs) const { return false; }
    
    Chunk& getChunk() { return *chunk; }
};

using Value = std::variant<double, bool, std::monostate, std::string, Function>;

class Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;

public:
    uint8_t getCode(int offset) const { return code[offset]; };
    void setCode(int offset, uint8_t value) { code[offset] = value; }
    const Value& getConstant(int constant) const { return constants[constant]; };
    void write(uint8_t byte, int line);
    void write(OpCode opcode, int line);
    unsigned long addConstant(Value value);
    int disassembleInstruction(int offset);
    void disassemble(const std::string& name);
    int getLine(int instruction) { return lines[instruction]; }
    int count() { return static_cast<int>(code.size()); }
};

struct OutputVisitor {
    void operator()(const double d) const { std::cout << d; }
    void operator()(const bool b) const { std::cout << (b ? "true" : "false"); }
    void operator()(const std::monostate n) const { std::cout << "nil"; }
    void operator()(const std::string& s) const { std::cout << s; }
    void operator()(const Function& f) const { std::cout << "<fn " << f.getName() << ">"; }
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
    bool operator()(const Function& f) const { return false; }
};

inline bool isFalsy(const Value& v) {
    return std::visit(FalsinessVisitor(), v);
}

#endif /* value_hpp */
