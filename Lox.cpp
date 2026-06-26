#include "Lox.h"
#include "Scanner.h"
#include "Parser.h"       
#include "ASTprinter.h"
#include "Interpreter.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

Interpreter Lox::interpreter;
bool Lox::hadError = false;

int Lox::mainProgram(int argc, char* argv[]) {
    if (argc > 2) {
        cout << "Usage: cpplox [script]" << endl;
        return 64;
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        runPrompt();
    }
    return 0;
}

void Lox::runFile(const string& path) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Could not open file: " << path << endl;
        exit(74);
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());

    if (hadError) exit(65);
}

void Lox::runPrompt() {
    cout << "Type 'exit' to quit." << endl;

    string line;
    
    for (;;) {
        cout << "> ";
        if (!getline(cin, line) || line == "exit") {
            break;
        }

        if (line.empty()) {
            continue;
        }

        run(line);
        hadError = false; 
    }
}

void Lox::run(const string& source) 
{
Scanner scanner(source);
vector<Token> tokens = scanner.scanTokens();

Parser parser(tokens);

auto expression = parser.parse();

if (hadError || !expression.has_value()) return;

Lox::interpreter.interpret(expression.value());
}

void Lox::error(int line, int column, const string& message) 
{
    report(line, column, "", message);
}

void Lox::error(Token token, const string& message) {
    if (token.type == TokenType::EOF_TOKEN) {
        report(token.line, token.column, " at end", message);
    } else {
        report(token.line, token.column, " at '" + token.lexeme + "'", message);
    }
}

void Lox::report(int line, int column, const string& where, const string& message) {
    cerr << "[line " << line << ":" << column << "] Error" << where << ": " << message << endl;
    hadError = true;
}