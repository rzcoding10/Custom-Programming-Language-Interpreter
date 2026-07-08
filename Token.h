#pragma once
#include <string>
#include <variant>
#include <unordered_map> 
#include <sstream>      
#include <iostream>
#include <utility>      
#include <type_traits>
#include <memory>       

using namespace std;
class LoxCallable;
class LoxInstance;

enum class TokenType
{
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,

    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,

    BANG,
    BANG_EQUAL,

    EQUAL,
    EQUAL_EQUAL,

    GREATER,
    GREATER_EQUAL,

    LESS,
    LESS_EQUAL,

    IDENTIFIER,
    STRING,
    NUMBER,

    AND,
    CLASS,
    ELSE,
    FALSE,
    FUN,
    FOR,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    EOF_TOKEN
};

inline const unordered_map<string, TokenType> keywords =
{
    {"and",    TokenType::AND},
    {"class",  TokenType::CLASS},
    {"else",   TokenType::ELSE},
    {"false",  TokenType::FALSE},
    {"for",    TokenType::FOR},
    {"fun",    TokenType::FUN},
    {"if",     TokenType::IF},
    {"nil",    TokenType::NIL},
    {"or",     TokenType::OR},
    {"print",  TokenType::PRINT},
    {"return", TokenType::RETURN},
    {"super",  TokenType::SUPER},
    {"this",   TokenType::THIS},
    {"true",   TokenType::TRUE},
    {"var",    TokenType::VAR},
    {"while",  TokenType::WHILE}
};
using Literal = variant<nullptr_t, double, string, bool, std::shared_ptr<LoxCallable>, std::shared_ptr<LoxInstance>>;

class Token
{
public:
    const TokenType type;
    const string lexeme;
    const Literal literal;
    const int line;
    const int column;

    Token(TokenType type,
          const string& lexeme,
          Literal literal,
          int line,
          int column)
        : type(type),
          lexeme(lexeme),
          literal(move(literal)),
          line(line),
          column(column)
    {
    }

    string toString() const
    {
        return tokenTypeToString(type)
               + " "
               + lexeme
               + " "
               + literalToString();
    }

private:
    string literalToString() const
    {
        return visit(
            [](const auto& value) -> string
            {
                using T = decay_t<decltype(value)>;

                if constexpr (is_same_v<T, nullptr_t>)
                {
                    return "nil";
                }
                else if constexpr (is_same_v<T, bool>)
                {
                    return value ? "true" : "false";
                }
                else if constexpr (is_same_v<T, string>)
                {
                    return value;
                }
                else if constexpr (is_same_v<T, std::shared_ptr<LoxCallable>> || 
                                   is_same_v<T, std::shared_ptr<LoxInstance>>)
                {
                    return "runtime_object";
                }
                else
                {
                    ostringstream oss;
                    oss << value;
                    return oss.str();
                }
            },
            literal);
    }

    static string tokenTypeToString(TokenType type)
    {
        switch (type)
        {
            case TokenType::LEFT_PAREN: return "LEFT_PAREN";
            case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
            case TokenType::LEFT_BRACE: return "LEFT_BRACE";
            case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";

            case TokenType::COMMA: return "COMMA";
            case TokenType::DOT: return "DOT";
            case TokenType::MINUS: return "MINUS";
            case TokenType::PLUS: return "PLUS";
            case TokenType::SEMICOLON: return "SEMICOLON";
            case TokenType::SLASH: return "SLASH";
            case TokenType::STAR: return "STAR";

            case TokenType::BANG: return "BANG";
            case TokenType::BANG_EQUAL: return "BANG_EQUAL";

            case TokenType::EQUAL: return "EQUAL";
            case TokenType::EQUAL_EQUAL: return "EQUAL_EQUAL";

            case TokenType::GREATER: return "GREATER";
            case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";

            case TokenType::LESS: return "LESS";
            case TokenType::LESS_EQUAL: return "LESS_EQUAL";

            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::STRING: return "STRING";
            case TokenType::NUMBER: return "NUMBER";

            case TokenType::AND: return "AND";
            case TokenType::CLASS: return "CLASS";
            case TokenType::ELSE: return "ELSE";
            case TokenType::FALSE: return "FALSE";
            case TokenType::FUN: return "FUN";
            case TokenType::FOR: return "FOR";
            case TokenType::IF: return "IF";
            case TokenType::NIL: return "NIL";
            case TokenType::OR: return "OR";
            case TokenType::PRINT: return "PRINT";
            case TokenType::RETURN: return "RETURN";
            case TokenType::SUPER: return "SUPER";
            case TokenType::THIS: return "THIS";
            case TokenType::TRUE: return "TRUE";
            case TokenType::VAR: return "VAR";
            case TokenType::WHILE: return "WHILE";

            case TokenType::EOF_TOKEN: return "EOF";
        }

        return "UNKNOWN";
    }
};

inline ostream& operator<<(ostream& os, const Token& token)
{
    os << token.toString();
    return os;
}