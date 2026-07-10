#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "Token.h"
#include "RuntimeError.h"
#include "Expr.h"

class Environment {
private:
    std::unordered_map<std::string, Literal> values;

    Environment* ancestor(int distance) {
        Environment* environment = this;
        for (int i = 0; i < distance; ++i) {
            environment = environment->enclosing.get(); 
        }
        return environment;
    }

public:
    std::shared_ptr<Environment> enclosing;
    
    Environment() : enclosing(nullptr) {}
    Environment(std::shared_ptr<Environment> enclosing) : enclosing(std::move(enclosing)) {} // <-- Fixed here!

    Literal getAt(int distance, const Token& name) {
        return ancestor(distance)->values[name.lexeme];
    }

    void assignAt(int distance, const Token& name, Literal value) {
        ancestor(distance)->values[name.lexeme] = value;
    }

    void define(const std::string& name, Literal value) {
        values[name] = value;
    }

    Literal get(const Token& name) {
        if (values.find(name.lexeme) != values.end()) {
            return values[name.lexeme];
        }

        if (enclosing != nullptr) {
            return enclosing->get(name);
        }

        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }

    void assign(const Token& name, Literal value) {
        if (values.find(name.lexeme) != values.end()) {
            values[name.lexeme] = value;
            return;
        }

        if (enclosing != nullptr) {
            enclosing->assign(name, value);
            return;
        }

        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }
};