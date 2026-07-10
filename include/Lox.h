#pragma once
#include "Token.h"
#include "Interpreter.h"
#include <string>

class Lox {
public:
    static bool hadError;
    static Interpreter interpreter;
    static int mainProgram(int argc, char* argv[]);
    static void runFile(const std::string& path);
    static void runPrompt();
    static void run(const std::string& source);
    static void error(int line, int column, const std::string& message);
    static void error(Token token, const std::string& message);
    static void report(int line, int column, const std::string& where, const std::string& message);
};