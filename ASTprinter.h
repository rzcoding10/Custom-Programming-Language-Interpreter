#pragma once
#include "Expr.h"
#include <iostream>
#include <string>
#include <variant>

using namespace std;

class AstPrinter 
{
public:
    string print(const Expr& expr) 
    {
        return visit([this](const auto& node) -> string 
        {
            using T = decay_t<decltype(*node)>;

            if constexpr (is_same_v<T, Binary>) 
            {
                return parenthesize(node->op.lexeme, node->left, node->right);
            } 
            else if constexpr (is_same_v<T, Grouping>) 
            {
                return parenthesize("group", node->expression);
            } 
            else if constexpr (is_same_v<T, LiteralExpr>) 
            {
                return visit([](const auto& val) -> string {
                using ValT = decay_t<decltype(val)>;
        
                if constexpr (is_same_v<ValT, nullptr_t>) return "nil";
                else if constexpr (is_same_v<ValT, string>) return val;
                else if constexpr (is_same_v<ValT, bool>) return val ? "true" : "false";
                else if constexpr (is_same_v<ValT, double>) return to_string(val);
                else return "<native fn>"; }, node->value);
            }
            else if constexpr (is_same_v<T, Unary>) 
            {
                return parenthesize(node->op.lexeme, node->right);
            }
            
            else if constexpr (is_same_v<T, Variable>)
            {
                return node->name.lexeme;
            }
            else if constexpr (is_same_v<T, Assign>)
            {
                return parenthesize("= " + node->name.lexeme, node->value);
            }
            
            return "Unknown Expression";

        }, expr);
    }

private:
    template <typename... Args>
    string parenthesize(const string& name, const Args&... exprs) 
    {
        string result = "(" + name;
        ((result += " " + print(exprs)), ...);
        result += ")";
        return result;
    }
};