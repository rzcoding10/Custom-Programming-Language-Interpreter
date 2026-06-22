#pragma once
#include <string>
#include "Token.h"

using namespace std;

class Lox {
public:
    static bool hadError;
    static int mainProgram(int argc, char* argv[]);
    static void runFile(const string& path);
    static void runPrompt();
    static void run(const string& source);
    static void error(int line, int column, const string& message);
    static void error(Token token, const string& message);
    static void report(int line, int column, const string& where, const string& message);
};