#pragma once
#include "Token.h"
#include "Expr.h"
#include <vector>
#include <stdexcept>
#include <optional>
#include <initializer_list>
#include <string>

using namespace std;

class Parser 
{
private:
    const vector<Token> tokens;
    int current = 0;

    class ParseError : public runtime_error {
    public:
        ParseError() : runtime_error("Parse Error") {}
    };

    Expr expression();
    Expr equality();
    Expr comparison();
    Expr term();
    Expr factor();
    Expr unary();
    Expr primary();

    bool match(initializer_list<TokenType> types);
    bool check(TokenType type);
    Token advance();
    bool isAtEnd();
    Token peek();
    Token previous();
    void synchronize();

    Token consume(TokenType type, const string& message);
    ParseError error(Token token, const string& message);

public:
    Parser(const vector<Token>& tokens);

    optional<Expr> parse();
};