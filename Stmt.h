#pragma once
#include <memory>
#include <variant>
#include <vector>
#include<optional>
#include "Expr.h"
#include "Token.h"

using namespace std;

struct Block;
struct ExpressionStmt;
struct PrintStmt;
struct VarStmt;


using Stmt = variant<
    unique_ptr<Block>,
    unique_ptr<ExpressionStmt>,
    unique_ptr<PrintStmt>,
    unique_ptr<VarStmt>
>;


struct Block {
    vector<Stmt> statements;

    Block(vector<Stmt> statements)
        : statements(std::move(statements)) {}
};

struct ExpressionStmt {
    Expr expression;

    ExpressionStmt(Expr expression)
        : expression(std::move(expression)) {}
};

struct PrintStmt {
    Expr expression;

    PrintStmt(Expr expression)
        : expression(std::move(expression)) {}
};

struct VarStmt {
    Token name;
    std::optional<Expr> initializer;

    VarStmt(Token name, std::optional<Expr> initializer)
        : name(std::move(name)), initializer(std::move(initializer)) {}
};