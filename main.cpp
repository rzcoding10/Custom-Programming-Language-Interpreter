#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <variant>
#include <sstream>
#include <utility>
#include <cstddef>
#include <unordered_map>
#include <string>
#include <type_traits>

using namespace std;

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

const unordered_map<string, TokenType> keywords =
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

using Literal = variant<nullptr_t,double,string,bool>;

class Token
{
public:
    const TokenType type;
    const string lexeme;
    const Literal literal;
    const int line;

    Token(TokenType type,
          const string& lexeme,
          Literal literal,
          int line)
        : type(type),
          lexeme(lexeme),
          literal(move(literal)),
          line(line)
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

class Lox 
{
public:
    static bool hadError;
    static void error(int line, const string& message);
    static void report(int line, const string& where, const string& message);
    static void run(const string& source);
    static void runFile(const string& path);
    static void runPrompt();
    static int mainProgram(int argc, char* argv[]);
};

class Scanner
{
private:
    const string source;
    vector<Token> tokens;
    size_t start = 0;
    size_t current = 0;
    int line = 1;

    bool isAtEnd()
    {
        return current >= source.length();
    }

    char advance()
    {
        return source[current++];
    } 

    bool match(char expected)
    {
        if (isAtEnd())
        {
            return false;
        }

        if (source[current] != expected)
        {
            return false;
        }

        current++;
        return true;
    }

    char peek()
    {
        if (isAtEnd())
        {
            return '\0';
        }

        return source[current];
    }

    char peekNext()
    {
        if (current + 1 >= source.length())
        {
            return '\0';
        }
        
        return source[current + 1];
    }

    bool isDigit(char c) 
    {
        return c >= '0' && c <= '9';
    }

    bool isAlpha(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    bool isAlphaNumeric(char c)
    {
        return isAlpha(c) || isDigit(c);
    }
    
    void number()
    {
        while (isDigit(peek()))
        {
            advance();
        }

        if (peek() == '.' && isDigit(peekNext()))
        {
            advance();
            while (isDigit(peek()))
            {
                advance();
            }
        }

        addToken(TokenType::NUMBER,stod(source.substr(start, current - start)));
    }

    void identifier()
    {
        while (isAlphaNumeric(peek()))
        {
            advance();
        }

        string text = source.substr(start, current - start);

        auto it = keywords.find(text);

        TokenType type;

        if (it == keywords.end())
        {
            type = TokenType::IDENTIFIER;
        }
        else
        {
            type = it->second;
        }
        
        addToken(type);
    }

    void scanString()
    {
        while (peek() != '"' && !isAtEnd())
        {
            if (peek() == '\n')
            {
                line++;
            }

            advance();
        }
        
        if (isAtEnd())
        {
            Lox::error(line, "Unterminated string.");
            return;
        }
        
        advance();
        string value = source.substr(start + 1,current - start - 2);

        addToken(TokenType::STRING, value);
    }

    void addToken(TokenType type)
    {
        addToken(type, nullptr);
    }

    void addToken(TokenType type, Literal literal)
    {
        string text = source.substr(start, current - start);

        tokens.push_back(
            Token(
                type,
                text,
                move(literal),
                line
            )
        );
    }

    void scanToken()
    {
    char c = advance();

    switch (c)
    {
        case '(':
            addToken(TokenType::LEFT_PAREN);
            break;

        case ')':
            addToken(TokenType::RIGHT_PAREN);
            break;

        case '{':
            addToken(TokenType::LEFT_BRACE);
            break;

        case '}':
            addToken(TokenType::RIGHT_BRACE);
            break;

        case ',':
            addToken(TokenType::COMMA);
            break;

        case '.':
            addToken(TokenType::DOT);
            break;

        case '-':
            addToken(TokenType::MINUS);
            break;

        case '+':
            addToken(TokenType::PLUS);
            break;

        case ';':
            addToken(TokenType::SEMICOLON);
            break;

        case '*':
            addToken(TokenType::STAR);
            break;
        
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;

        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;

        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;

        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;

        case '/':
            if (match('/'))
            {
                while (peek() != '\n' && !isAtEnd())
                {
                    advance();
                }
            }
            else
            {
                addToken(TokenType::SLASH);
            }
            break;

        case ' ':
        case '\r':
        case '\t':
            break;

        case '\n':
            line++;
            break;

        case '"':
           scanString();
           break;
        
        default:
            if (isDigit(c)) 
            {
                number();
            }
            else if (isAlpha(c))
            {
                identifier();
            }
            else
            {
                Lox::error(line, "Unexpected character.");
                break;
            }
    }
    }

public:
    Scanner(const string& source): source(source)
    {

    }

    vector<Token> scanTokens()
    {
        while (!isAtEnd())
        {
            start = current;
            scanToken();
        }

        tokens.push_back(
            Token(
                TokenType::EOF_TOKEN,
                "",
                nullptr,
                line
            )
        );

        return tokens;
    }
};

bool Lox::hadError = false;

void Lox::error(int line, const string& message)
{
    report(line, "", message);
}

void Lox::report(int line, const string& where, const string& message)
{
    cerr << "[line " << line << "] Error" << where << ": " << message << endl;
    hadError = true;
}

void Lox::run(const string& source) 
{
    Scanner scanner(source);
    vector<Token> tokens = scanner.scanTokens();

    for (const Token& token : tokens) 
    {
        cout << token << endl;
    }
}

void Lox::runFile(const string& path) 
{
    ifstream file(path);

    if (!file.is_open()) 
    {
        cerr << "Could not open file." << endl;
        return;
    }

    ostringstream buffer;
    buffer << file.rdbuf();
    string source = buffer.str();

    run(source);

    if (hadError) exit(65);
}

void Lox::runPrompt() 
{
    string line;
    while (true) 
    {
        cout << "> ";
        getline(cin, line);
        if (cin.eof()) break;

        run(line);
        hadError = false;
    }
}

int Lox::mainProgram(int argc, char* argv[]) 
{
    if (argc > 2) 
    {
        cout << "Usage: clox [script]" << endl;
        return 64;
    }
    else if (argc == 2) 
    {
        runFile(argv[1]);
    }
    else 
    {
        runPrompt();
    }
    return 0;
}

int main(int argc, char* argv[]) 
{
    return Lox::mainProgram(argc, argv);
}