#pragma once
#include <string>

using namespace std;

class Lox {
public:
    static bool hadError;
    static int mainProgram(int argc, char* argv[]);
    static void runFile(const string& path);
    static void runPrompt();
    static void run(const string& source);
    static void error(int line, const string& message);
    static void report(int line, const string& where, const string& message);
};