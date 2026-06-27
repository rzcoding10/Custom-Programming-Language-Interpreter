#pragma once
#include "Expr.h"
#include <memory>
#include <variant>
#include <utility>
#include <optional>

using namespace std;

struct ExpressionStmt 
{
    Expr expression;

    ExpressionStmt(Expr expr) : expression(move(expr)) {}
};

struct PrintStmt 
{
    Expr expression;

    PrintStmt(Expr expr) : expression(move(expr)) {}
};

struct VarStmt {
    Token name;
    std::optional<Expr> initializer;

    VarStmt(Token name, std::optional<Expr> initializer) 
        : name(std::move(name)), initializer(std::move(initializer)) {}
};

using Stmt = variant<
    unique_ptr<ExpressionStmt>,
    unique_ptr<PrintStmt>,
    unique_ptr<VarStmt>
>;