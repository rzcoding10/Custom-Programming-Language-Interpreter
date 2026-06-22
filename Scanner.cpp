#include "Scanner.h"



Scanner::Scanner(const string& source) : source(source) {}

bool Scanner::isAtEnd()
{
    return current >= source.length();
}

char Scanner::advance()
{
    currentcolumn++;
    return source[current++];
} 

bool Scanner::match(char expected)
{
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

char Scanner::peek()
{
    if (isAtEnd()) return '\0';
    return source[current];
}

char Scanner::peekNext()
{
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

bool Scanner::isDigit(char c) 
{
    return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::isAlphaNumeric(char c)
{
    return isAlpha(c) || isDigit(c);
}

void Scanner::number()
{
    while (isDigit(peek())) advance();

    if (peek() == '.' && isDigit(peekNext()))
    {
        advance();
        while (isDigit(peek())) advance();
    }

    addToken(TokenType::NUMBER, stod(source.substr(start, current - start)));
}

void Scanner::identifier()
{
    while (isAlphaNumeric(peek())) advance();

    string text = source.substr(start, current - start);
    auto it = keywords.find(text);
    TokenType type = (it == keywords.end()) ? TokenType::IDENTIFIER : it->second;
    
    addToken(type);
}

void Scanner::scanString()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n') line++;
        advance();
    }
    
    if (isAtEnd())
    {
        Lox::error(line, currentcolumn, "Unexpected character.");
        return;
    }
    
    advance();
    string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING, value);
}

void Scanner::addToken(TokenType type)
{
    addToken(type, nullptr);
}

void Scanner::addToken(TokenType type, Literal literal)
{
    string text = source.substr(start, current - start);
    int startcolumn = currentcolumn - (current - start);
    tokens.push_back(Token(type, text, move(literal), line, startcolumn));
}

void Scanner::scanToken()
{
    char c = advance();
    switch (c)
    {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case '-': addToken(TokenType::MINUS); break;
        case '+': addToken(TokenType::PLUS); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;
        case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
        case '/':
            if (match('/')) {
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::SLASH);
            }
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            line++;
            currentcolumn=1;
            break;
        case '"':
           scanString();
           break;
        default:
            if (isDigit(c)) {
                number();
            } else if (isAlpha(c)) {
                identifier();
            } else {
                Lox::error(line, currentcolumn, "Unterminated string.");
            }
            break;
    }
}

vector<Token> Scanner::scanTokens()
{
    while (!isAtEnd())
    {
        start = current;
        scanToken();
    }

    tokens.push_back(Token(TokenType::EOF_TOKEN, "", nullptr, line, currentcolumn));
    return tokens;
}