#pragma once
#include "Token.h"
#include <memory>
#include <vector>

using namespace std;

struct Binary;
struct Grouping;
struct LiteralExpr;
struct Unary;
struct Variable;


using Expr = variant<
    unique_ptr<Binary>,
    unique_ptr<Grouping>,
    unique_ptr<LiteralExpr>,
    unique_ptr<Unary>,
    unique_ptr<Variable>
>;


template <typename T, typename... Args>
Expr makeExpr(Args&&... args)
{
    return make_unique<T>(forward<Args>(args)...);
}


struct Binary
{
    Expr left;
    Token op;
    Expr right;

    Binary(Expr left, Token op, Expr right)
        : left(move(left)), op(move(op)), right(move(right)) {}
};

struct Grouping
{
    Expr expression;

    Grouping(Expr expression)
        : expression(move(expression)) {}
};

struct LiteralExpr
{
    Literal value;

    LiteralExpr(Literal value)
        : value(move(value)) {}
};

struct Unary
{
    Token op;
    Expr right;

    Unary(Token op, Expr right)
        : op(move(op)), right(move(right)) {}
};

struct Variable 
{
    Token name;

    Variable(Token name) : name(std::move(name)) {}
};