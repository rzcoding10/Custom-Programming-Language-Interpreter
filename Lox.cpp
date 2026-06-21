#include "Lox.h"
#include "Scanner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

bool Lox::hadError = false;

int Lox::mainProgram(int argc, char* argv[]) {
    if (argc > 2) {
        cout << "Usage: clox [script]" << endl;
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
    string line;
    for (;;) {
        cout << "> ";
        if (!getline(cin, line)) break;
        run(line);
        hadError = false; 
    }
}

void Lox::run(const string& source) {
    Scanner scanner(source);
    vector<Token> tokens = scanner.scanTokens();

    for (const Token& token : tokens) {
        cout << token << endl;
    }
}

void Lox::error(int line, const string& message) {
    report(line, "", message);
}

void Lox::report(int line, const string& where, const string& message) {
    cerr << "[line " << line << "] Error" << where << ": " << message << endl;
    hadError = true;
}