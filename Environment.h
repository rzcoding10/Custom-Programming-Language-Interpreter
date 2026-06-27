#pragma once
#include <unordered_map>
#include <string>
#include "Token.h"
#include "RuntimeError.h"
// Include wherever your 'Literal' type is defined (e.g., "Expr.h" or "Literal.h")

class Environment {
private:
    std::unordered_map<std::string, Literal> values;

public:
    // 1. Create or overwrite a variable
    void define(const std::string& name, Literal value) {
        values[name] = value;
    }

    // 2. Read a variable's value
    Literal get(const Token& name) {
        if (values.find(name.lexeme) != values.end()) {
            return values[name.lexeme];
        }

        // If the variable isn't in the map, the user messed up!
        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }
};