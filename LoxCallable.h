#pragma once
#include "Token.h"
#include <vector>
#include <string>
#include <memory>

class Interpreter; 

class LoxCallable {
public:
    virtual ~LoxCallable() = default;

    virtual int arity() = 0;
    virtual Literal call(Interpreter& interpreter, std::vector<Literal>& arguments) = 0;
    virtual std::string toString() = 0; 
};