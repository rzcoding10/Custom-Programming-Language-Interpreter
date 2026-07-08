#pragma once
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <memory>
#include "LoxCallable.h"

class LoxFunction; // Forward declaration

class LoxClass : public LoxCallable, public std::enable_shared_from_this<LoxClass> {
public:
    std::string name;
    std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods;

    LoxClass(std::string name, std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods) 
        : name(std::move(name)), methods(std::move(methods)) {}

    std::string toString() override {
        return name;
    }

    int arity() override {
        return 0; 
    }

    Literal call(Interpreter& interpreter, std::vector<Literal>& arguments) override;

    std::shared_ptr<LoxFunction> findMethod(const std::string& name) {
        if (methods.count(name)) {
            return methods.at(name);
        }
        return nullptr;
    }
};