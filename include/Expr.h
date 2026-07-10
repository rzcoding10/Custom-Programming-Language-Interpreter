#pragma once
#include "Token.h"
#include <memory>
#include <vector>
#include <variant>

struct Binary;
struct Grouping;
struct LiteralExpr;
struct Logical;
struct Unary;
struct Variable;
struct Assign;
struct Call;
struct Get;
struct Set;
struct This;
struct Super;

using Expr = std::variant<
    std::unique_ptr<Assign>,
    std::unique_ptr<Binary>,
    std::unique_ptr<Grouping>,
    std::unique_ptr<LiteralExpr>,
    std::unique_ptr<Logical>,
    std::unique_ptr<Unary>,
    std::unique_ptr<Variable>,
    std::unique_ptr<Call>,
    std::unique_ptr<Get>,
    std::unique_ptr<Set>,
    std::unique_ptr<This>,
    std::unique_ptr<Super>
>;

template <typename T, typename... Args>
Expr makeExpr(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

struct Binary {
    Expr left;
    Token op;
    Expr right;
    Binary(Expr left, Token op, Expr right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

struct Grouping {
    Expr expression;
    Grouping(Expr expression)
        : expression(std::move(expression)) {}
};

struct LiteralExpr {
    Literal value;
    LiteralExpr(Literal value)
        : value(std::move(value)) {}
};

struct Unary {
    Token op;
    Expr right;
    Unary(Token op, Expr right)
        : op(std::move(op)), right(std::move(right)) {}
};

struct Variable {
    Token name;
    Variable(Token name) : name(std::move(name)) {}
};

struct Assign {
    Token name;
    Expr value;
    Assign(Token name, Expr value)
        : name(std::move(name)), value(std::move(value)) {}
};

struct Logical {
    Expr left;
    Token op;
    Expr right;
    Logical(Expr left, Token op, Expr right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

struct Call {
    Expr callee;
    Token paren;
    std::vector<Expr> arguments;
    Call(Expr callee, Token paren, std::vector<Expr> arguments)
        : callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(arguments)) {}
};

struct Get {
    Expr object;  
    Token name;   
    Get(Expr object, Token name)
        : object(std::move(object)), name(std::move(name)) {}
};

struct Set {
    Expr object;  
    Token name;   
    Expr value;   
    Set(Expr object, Token name, Expr value)
        : object(std::move(object)), name(std::move(name)), value(std::move(value)) {}
};

struct This {
    Token keyword;
    This(Token keyword) : keyword(std::move(keyword)) {}
};

struct Super {
    Token keyword;
    Token method;
    Super(Token keyword, Token method)
        : keyword(std::move(keyword)), method(std::move(method)) {}
};

