#pragma once
#include "Token.h"
#include <memory>
#include <vector>

using namespace std;

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


using Expr = variant<
    unique_ptr<Assign>,
    unique_ptr<Binary>,
    unique_ptr<Grouping>,
    unique_ptr<LiteralExpr>,
    std::unique_ptr<Logical>,
    unique_ptr<Unary>,
    unique_ptr<Variable>,
    unique_ptr<Call>,
    unique_ptr<Get>,
    unique_ptr<Set>,
    unique_ptr<This>
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

struct Assign 
{
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
    Expr object;  // The expression on the left of the dot (e.g., 'bagel')
    Token name;   // The property name on the right of the dot (e.g., 'flavor')

    Get(Expr object, Token name)
        : object(std::move(object)), name(std::move(name)) {}
};

struct Set {
    Expr object;  // The expression on the left of the dot
    Token name;   // The property name
    Expr value;   // The value being assigned after the '='

    Set(Expr object, Token name, Expr value)
        : object(std::move(object)), name(std::move(name)), value(std::move(value)) {}
};

struct This {
    Token keyword;

    This(Token keyword) : keyword(std::move(keyword)) {}
};