#pragma once
#include "Token.h"
#include "Expr.h"
#include "Stmt.h"
#include <vector>
#include <stdexcept>
#include <optional>
#include <initializer_list>
#include <string>

class Parser 
{
private:
    const std::vector<Token> tokens;
    int current = 0;

    class ParseError : public std::runtime_error {
    public:
        ParseError() : std::runtime_error("Parse Error") {}
    };

    Stmt declaration();
    Stmt varDeclaration();
    Stmt statement();
    std::vector<Stmt> block();
    Stmt printStatement();
    Stmt expressionStatement();
    Stmt ifStatement();
    Stmt whileStatement();
    Stmt forStatement();
    Stmt function(std::string kind);
    Stmt returnStatement();
    Stmt classDeclaration();

    Expr expression();
    Expr assignment();
    Expr equality();
    Expr comparison();
    Expr term();
    Expr factor();
    Expr unary();
    Expr primary();
    Expr logic_or();
    Expr logic_and();
    Expr call();
    Expr finishCall(Expr callee);

    bool match(std::initializer_list<TokenType> types);
    bool check(TokenType type);
    Token advance();
    bool isAtEnd();
    Token peek();
    Token previous();
    void synchronize();

    Token consume(TokenType type, const std::string& message);
    ParseError error(Token token, const std::string& message);

public:
    Parser(const std::vector<Token>& tokens);

    std::vector<Stmt> parse();
};