#pragma once
#include "Token.h"
#include "Lox.h"
#include <string>
#include <vector>

using namespace std;

class Scanner
{
private:
    const string source;
    vector<Token> tokens;
    size_t start = 0;
    size_t current = 0;
    int line = 1;
    int currentcolumn = 1;

    bool isAtEnd();
    char advance();
    bool match(char expected);
    char peek();
    char peekNext();
    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlphaNumeric(char c);
    void number();
    void identifier();
    void scanString();
    void addToken(TokenType type);
    void addToken(TokenType type, Literal literal);
    void scanToken();

public:
    Scanner(const string& source);
    vector<Token> scanTokens();
};