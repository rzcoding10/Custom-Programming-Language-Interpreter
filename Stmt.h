#pragma once
#include <memory>
#include <variant>
#include <vector>
#include <optional>
#include "Expr.h"
#include "Token.h"

using namespace std;

struct Block;
struct ExpressionStmt;
struct IfStmt;
struct PrintStmt;
struct VarStmt;
struct WhileStmt;
struct FunctionStmt;
struct ReturnStmt;
struct ClassStmt;


using Stmt = variant<
    unique_ptr<Block>,
    unique_ptr<ClassStmt>,
    unique_ptr<ExpressionStmt>,
    unique_ptr<IfStmt>,
    unique_ptr<PrintStmt>,
    unique_ptr<VarStmt>,
    unique_ptr<WhileStmt>,
    unique_ptr<FunctionStmt>,
    unique_ptr<ReturnStmt>
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

struct IfStmt {
    Expr condition;
    Stmt thenBranch;
    std::optional<Stmt> elseBranch;

    IfStmt(Expr condition, Stmt thenBranch, std::optional<Stmt> elseBranch)
        : condition(std::move(condition)), 
          thenBranch(std::move(thenBranch)), 
          elseBranch(std::move(elseBranch)) {}
};

struct WhileStmt {
    Expr condition;
    Stmt body;

    WhileStmt(Expr condition, Stmt body)
        : condition(std::move(condition)), body(std::move(body)) {}
};

struct FunctionStmt {
    Token name;
    std::vector<Token> params;
    std::vector<Stmt> body;
};

struct ReturnStmt {
    Token keyword;              
    std::optional<Expr> value;  

    ReturnStmt(Token keyword, std::optional<Expr> value) 
        : keyword(std::move(keyword)), value(std::move(value)) {}
};

struct ClassStmt {
    Token name;
    // We store these specifically as FunctionStmts, not generic Stmts
    std::vector<std::unique_ptr<FunctionStmt>> methods;

    ClassStmt(Token name, std::vector<std::unique_ptr<FunctionStmt>> methods)
        : name(std::move(name)), methods(std::move(methods)) {}
};