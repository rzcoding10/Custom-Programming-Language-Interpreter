#pragma once
#include "Expr.h"
#include "Token.h"
#include <variant>
#include <type_traits>
#include "RuntimeError.h"
#include <iostream>

using namespace std;

class Interpreter 
{
private:
    void checkNumberOperand(const Token& op, const Literal& operand) {
        if (holds_alternative<double>(operand)) return;
        throw RuntimeError(op, "Operand must be a number.");
    }

    void checkNumberOperands(const Token& op, const Literal& left, const Literal& right) {
        if (holds_alternative<double>(left) && holds_alternative<double>(right)) return;
        throw RuntimeError(op, "Operands must be numbers.");
    }

    bool isTruthy(const Literal& object) 
    {
        if (holds_alternative<nullptr_t>(object)) return false;
        if (holds_alternative<bool>(object)) return get<bool>(object);
        return true;
    }
    
    bool isEqual(const Literal& a, const Literal& b) 
    {
        return a == b; 
    }
    
    string stringify(const Literal& value) 
    {
        return visit([](const auto& val) -> string {
            using ValT = decay_t<decltype(val)>;
            
            if constexpr (is_same_v<ValT, nullptr_t>) return "nil";
            else if constexpr (is_same_v<ValT, bool>) return val ? "true" : "false";
            else if constexpr (is_same_v<ValT, string>) return val;
            else if constexpr (is_same_v<ValT, double>) 
            {
                string text = to_string(val);
                text.erase(text.find_last_not_of('0') + 1, string::npos);
                if (text.back() == '.') text.pop_back();
                return text;
            }
        }, value);
    }

public:
    Literal evaluate(const Expr& expr) 
    {
        return visit([this](const auto& node) -> Literal 
        {
            using T = decay_t<decltype(*node)>;

            if constexpr (is_same_v<T, LiteralExpr>) 
            {
                return node->value;
            } 
            else if constexpr (is_same_v<T, Grouping>) 
            {
                return evaluate(node->expression);
            } 
            else if constexpr (is_same_v<T, Unary>) 
            {
                Literal right = evaluate(node->right);

                switch (node->op.type) 
                {
                    case TokenType::BANG:
                        return !isTruthy(right);
                    
                    case TokenType::MINUS:
                        checkNumberOperand(node->op, right);
                        return -get<double>(right);
                }

                return nullptr;
            }
            else if constexpr (is_same_v<T, Binary>) 
            {
                Literal left = evaluate(node->left);
                Literal right = evaluate(node->right);

                switch (node->op.type) 
                {
                    case TokenType::MINUS:
                        checkNumberOperands(node->op, left, right); // CHECK TYPES HERE
                        return get<double>(left) - get<double>(right);
                    case TokenType::SLASH:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) / get<double>(right);
                    case TokenType::STAR:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) * get<double>(right);
                    case TokenType::PLUS:
                        if (holds_alternative<double>(left) && holds_alternative<double>(right)) 
                        {
                            return get<double>(left) + get<double>(right);
                        }
                        if (holds_alternative<string>(left) && holds_alternative<string>(right)) 
                        {
                            return get<string>(left) + get<string>(right);
                        }
                        throw RuntimeError(node->op, "Operands must be two numbers or two strings.");

                    case TokenType::GREATER:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) > get<double>(right);
                    case TokenType::GREATER_EQUAL:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) >= get<double>(right);
                    case TokenType::LESS:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) < get<double>(right);
                    case TokenType::LESS_EQUAL:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) <= get<double>(right);

                    case TokenType::BANG_EQUAL:
                        return !isEqual(left, right);
                    case TokenType::EQUAL_EQUAL:
                        return isEqual(left, right);
                }

                return nullptr;
            }

        }, expr);
    }
    
    void interpret(const Expr& expression) 
    {
        try {
            Literal value = evaluate(expression);
            cout << stringify(value) << "\n";
        } catch (const RuntimeError& error) 
        {
            cerr << error.what() << "\n[line " << error.token.line << "]\n";
        }
    }
};