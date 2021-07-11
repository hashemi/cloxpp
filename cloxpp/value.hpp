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
#include <unordered_map>

struct NativeFunctionObject;
struct UpvalueObject;
struct ClassObject;
struct InstanceObject;
struct BoundMethodObject;
class FunctionObject;
class ClosureObject;
class Compiler;
class Parser;
class VM;
using Function = std::shared_ptr<FunctionObject>;
using NativeFunction = std::shared_ptr<NativeFunctionObject>;
using Closure = std::shared_ptr<ClosureObject>;
using UpvalueValue = std::shared_ptr<UpvalueObject>;
using ClassValue = std::shared_ptr<ClassObject>;
using InstanceValue = std::shared_ptr<InstanceObject>;
using BoundMethodValue = std::shared_ptr<BoundMethodObject>;

using Value = std::variant<double, bool, std::monostate, std::string, Function, NativeFunction, Closure, UpvalueValue, ClassValue, InstanceValue, BoundMethodValue>;

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

typedef Value (*NativeFn)(int argCount, std::vector<Value>::iterator args);

struct NativeFunctionObject {
    NativeFn function;
};

struct UpvalueObject {
    Value* location;
    Value closed;
    UpvalueValue next;
    UpvalueObject(Value* slot): location(slot), closed(std::monostate()), next(nullptr) {}
};

struct ClassObject {
    std::string name;
    std::unordered_map<std::string, Closure> methods;
    explicit ClassObject(std::string name): name(name) {}
};

struct InstanceObject {
    ClassValue klass;
    std::unordered_map<std::string, Value> fields;
    explicit InstanceObject(ClassValue klass): klass(klass) {}
};

struct BoundMethodObject {
    InstanceValue receiver;
    Closure method;
    explicit BoundMethodObject(InstanceValue receiver, Closure method)
        : receiver(receiver), method(method) {}
};

class FunctionObject {
private:
    int arity;
    int upvalueCount = 0;
    std::string name;
    Chunk chunk;

public:
    FunctionObject(int arity, const std::string& name)
        : arity(arity), name(name), chunk(Chunk()) {}
    
    const std::string& getName() const { return name; }

    bool operator==(const Function& rhs) const { return false; }
    
    Chunk& getChunk() { return chunk; }
    uint8_t getCode(int offset) { return chunk.getCode(offset); }
    const Value& getConstant(int constant) const { return chunk.getConstant(constant); }

    friend Compiler;
    friend Parser;
    friend VM;
    friend Chunk;
    friend ClosureObject;
};

class ClosureObject {
public:
    Function function;
    std::vector<UpvalueValue> upvalues;
    explicit ClosureObject(Function function): function(function) {
        upvalues.resize(function->upvalueCount, nullptr);
    };
};

std::ostream& operator<<(std::ostream& os, const Value& v);

struct OutputVisitor {
    void operator()(const double d) const { std::cout << d; }
    void operator()(const bool b) const { std::cout << (b ? "true" : "false"); }
    void operator()(const std::monostate n) const { std::cout << "nil"; }
    void operator()(const std::string& s) const { std::cout << s; }
    void operator()(const Function& f) const {
        if (f->getName().empty()) {
            std::cout << "<script>";
        } else {
            std::cout << "<fn " << f->getName() << ">";
        }
    }
    void operator()(const NativeFunction& f) const { std::cout << "<native fn>"; }
    void operator()(const Closure& c) const { std::cout << Value(c->function); }
    void operator()(const UpvalueValue& u) const { std::cout << "upvalue"; }
    void operator()(const ClassValue& c) const { std::cout << c->name; }
    void operator()(const InstanceValue& i) const { std::cout << i->klass->name << " instance"; }
    void operator()(const BoundMethodValue& m) const { std::cout << Value(m->method->function); }
};

inline std::ostream& operator<<(std::ostream& os, const Value& v) {
    std::visit(OutputVisitor(), v);
    return os;
}

struct FalsinessVisitor {
    bool operator()(const bool b) const { return !b; }
    bool operator()(const std::monostate n) const { return true; }
    
    template <typename T>
    bool operator()(const T& value) const { return false; }
};

inline bool isFalsy(const Value& v) {
    return std::visit(FalsinessVisitor(), v);
}

#endif /* value_hpp */
