#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "LoxClass.h"
#include "RuntimeError.h"

class LoxInstance : public std::enable_shared_from_this<LoxInstance> {
private:
    std::shared_ptr<LoxClass> klass;
    std::unordered_map<std::string, Literal> fields;

public:
    LoxInstance(std::shared_ptr<LoxClass> klass) : klass(std::move(klass)) {}

    Literal get(const Token& name);
    void set(const Token& name, Literal value);

    std::string toString() const {
        return klass->name + " instance";
    }
};