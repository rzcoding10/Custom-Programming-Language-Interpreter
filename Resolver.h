#pragma once
#include <vector>
#include <map>
#include <string>
#include <variant>
#include <memory>
#include "Expr.h"
#include "Stmt.h"
#include "Interpreter.h"
#include "Token.h"
#include "Lox.h"


class Resolver {
private:
    Interpreter& interpreter;
    
    // The simulated scope stack. 
    // bool = true means the variable is fully defined and ready to use.
    std::vector<std::map<std::string, bool>> scopes;

    // Prevents returning from outside of a function
    enum class FunctionType { NONE, FUNCTION };
    FunctionType currentFunction = FunctionType::NONE;

public:
    Resolver(Interpreter& interpreter) : interpreter(interpreter) {}

    // Main entry point for a vector of statements (e.g., from the parser)
    void resolve(const std::vector<Stmt>& statements) {
        for (const auto& statement : statements) {
            resolve(statement);
        }
    }

    // Resolves a Statement variant using std::visit
    void resolve(const Stmt& stmt) {
        std::visit([this](const auto& n) { this->visit(n); }, stmt);
    }

    // Resolves an Expression variant using std::visit
    void resolve(const Expr& expr) {
        std::visit([this](const auto& n) { this->visit(n); }, expr);
    }

private:
    void beginScope() {
        scopes.push_back(std::map<std::string, bool>());
    }

    void endScope() {
        scopes.pop_back();
    }

    void declare(const Token& name) {
        if (scopes.empty()) return;
        
        std::map<std::string, bool>& scope = scopes.back();
        
        // If you have an error reporter, you can uncomment this to catch duplicate variables
        if (scope.count(name.lexeme)) {
            Lox::error(name, "Already a variable with this name in this scope.");
        }
        
        scope[name.lexeme] = false; // Exists, but not ready yet
    }

    void define(const Token& name) {
        if (scopes.empty()) return;
        scopes.back()[name.lexeme] = true; // Fully ready to be used
    }

    // Pass the raw pointer of the AST node to the interpreter
    template <typename T>
    void resolveLocal(const T* expr, const Token& name) {
        // Walk BACKWARDS through the scopes to find the closest matching variable
        for (int i = scopes.size() - 1; i >= 0; --i) {
            if (scopes[i].count(name.lexeme)) {
                // Pass the raw memory address and the depth to the interpreter
                interpreter.resolve(expr, scopes.size() - 1 - i);
                return;
            }
        }
        // If not found, assume it is a global variable.
    }

    void resolveFunction(const FunctionStmt* function, FunctionType type) {
        FunctionType enclosingFunction = currentFunction;
        currentFunction = type;

        beginScope();
        for (const auto& param : function->params) {
            declare(param);
            define(param);
        }
        
        resolve(function->body);
        endScope();

        currentFunction = enclosingFunction;
    }

    // ==========================================
    // VISITOR METHODS FOR STATEMENTS
    // ==========================================

    void visit(const std::unique_ptr<Block>& stmt) {
        beginScope();
        resolve(stmt->statements);
        endScope();
    }

    void visit(const std::unique_ptr<ExpressionStmt>& stmt) {
        resolve(stmt->expression);
    }

    void visit(const std::unique_ptr<IfStmt>& stmt) {
        resolve(stmt->condition);
        resolve(stmt->thenBranch);
        if (stmt->elseBranch.has_value()) {
            resolve(stmt->elseBranch.value());
        }
    }

    void visit(const std::unique_ptr<PrintStmt>& stmt) {
        resolve(stmt->expression);
    }

    void visit(const std::unique_ptr<VarStmt>& stmt) {
        declare(stmt->name);
        if (stmt->initializer.has_value()) {
            resolve(stmt->initializer.value());
        }
        define(stmt->name);
    }

    void visit(const std::unique_ptr<WhileStmt>& stmt) {
        resolve(stmt->condition);
        resolve(stmt->body);
    }

    void visit(const std::unique_ptr<FunctionStmt>& stmt) {
        declare(stmt->name);
        define(stmt->name);
        resolveFunction(stmt.get(), FunctionType::FUNCTION);
    }

    void visit(const std::unique_ptr<ReturnStmt>& stmt) {
        if (currentFunction == FunctionType::NONE) {
            Lox::error(stmt->keyword, "Can't return from top-level code.");
        }
        if (stmt->value.has_value()) {
            resolve(stmt->value.value());
        }
    }


    // ==========================================
    // VISITOR METHODS FOR EXPRESSIONS
    // ==========================================

    void visit(const std::unique_ptr<Assign>& expr) {
        resolve(expr->value);
        resolveLocal(expr.get(), expr->name);
    }

    void visit(const std::unique_ptr<Binary>& expr) {
        resolve(expr->left);
        resolve(expr->right);
    }

    void visit(const std::unique_ptr<Call>& expr) {
        resolve(expr->callee);
        for (const auto& arg : expr->arguments) {
            resolve(arg);
        }
    }

    void visit(const std::unique_ptr<Grouping>& expr) {
        resolve(expr->expression);
    }

    void visit(const std::unique_ptr<LiteralExpr>& expr) {
        // A literal doesn't mention any variables, so there is nothing to resolve.
    }

    void visit(const std::unique_ptr<Logical>& expr) {
        resolve(expr->left);
        resolve(expr->right);
    }

    void visit(const std::unique_ptr<Unary>& expr) {
        resolve(expr->right);
    }

    void visit(const std::unique_ptr<Variable>& expr) {
        if (!scopes.empty() && 
            scopes.back().count(expr->name.lexeme) && 
            scopes.back()[expr->name.lexeme] == false) {
            Lox::error(expr->name, "Can't read local variable in its own initializer.");
        }
        resolveLocal(expr.get(), expr->name);
    }
};