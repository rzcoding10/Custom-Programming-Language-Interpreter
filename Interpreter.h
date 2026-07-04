#pragma once
#include "Expr.h"
#include "Token.h"
#include "Stmt.h"
#include "RuntimeError.h"
#include "Environment.h"
#include "LoxCallable.h"
#include <variant>
#include <type_traits>
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <exception>
#include <map>

class Interpreter;
using namespace std;

class ReturnException : public std::exception {
public:
    const Literal value;
    
    ReturnException(Literal value) : value(value) {}
};

class ClockCallable : public LoxCallable {
public:
    int arity() override { 
        return 0; 
    }
    
    Literal call(Interpreter& interpreter, std::vector<Literal>& arguments) override;

    std::string toString() override { 
        return "<native fn>"; 
    }
};

class LoxFunction : public LoxCallable 
{
private:
    FunctionStmt* declaration;
    std::shared_ptr<Environment> closure;

public:
    LoxFunction(FunctionStmt* declaration, std::shared_ptr<Environment> closure) 
        : declaration(declaration), closure(std::move(closure)) {}
        
    int arity() override { return declaration->params.size(); }
    
    Literal call(Interpreter& interpreter, std::vector<Literal>& arguments) override;
    
    std::string toString() override { return "<fn " + declaration->name.lexeme + ">"; }
};

class Interpreter 
{
public:
    std::shared_ptr<Environment> globals;
    
private:
    std::shared_ptr<Environment> environment;
    std::map<const void*, int> locals;

public:
    Interpreter() 
    {
        globals = std::make_shared<Environment>();
        environment = globals;
        globals->define("clock", std::make_shared<ClockCallable>());
    }

    void resolve(const void* expr, int depth) {
        locals[expr] = depth;
    }
private:
    Literal lookUpVariable(const Token& name, const void* expr) {
        auto it = locals.find(expr);
        if (it != locals.end()) {
            int distance = it->second;
            return environment->getAt(distance, name);
        } else {
            return globals->get(name);
        }
    }

    void checkNumberOperand(const Token& op, const Literal& operand) {
        if (holds_alternative<double>(operand)) return;
        throw RuntimeError(op, "Operand must be a number.");
    }

    void checkNumberOperands(const Token& op, const Literal& left, const Literal& right) {
        if (holds_alternative<double>(left) && holds_alternative<double>(right)) return;
        throw RuntimeError(op, "Operands must be numbers.");
    }

    bool isTruthy(const Literal& object) 
    {
        if (holds_alternative<nullptr_t>(object)) return false;
        if (holds_alternative<bool>(object)) return get<bool>(object);
        return true;
    }
    
    bool isEqual(const Literal& a, const Literal& b) 
    {
        return a == b; 
    }
    
    string stringify(const Literal& value) 
    {
        return visit([](const auto& val) -> string {
            using ValT = decay_t<decltype(val)>;
            
            if constexpr (is_same_v<ValT, nullptr_t>) return "nil";
            else if constexpr (is_same_v<ValT, bool>) return val ? "true" : "false";
            else if constexpr (is_same_v<ValT, string>) return val;
            else if constexpr (is_same_v<ValT, double>) 
            {
                string text = to_string(val);
                text.erase(text.find_last_not_of('0') + 1, string::npos);
                if (text.back() == '.') text.pop_back();
                return text;
            }
            else if constexpr (is_same_v<ValT, std::shared_ptr<LoxCallable>>) 
            {
                return val->toString();
            }
            return "";
        }, value);
    }

    void execute(const Stmt& stmt) 
    {
        visit([this](const auto& node) {
            using T = decay_t<decltype(*node)>;

            if constexpr (is_same_v<T, PrintStmt>) {
                Literal value = evaluate(node->expression);
                cout << stringify(value) << "\n";
            } 
            else if constexpr (is_same_v<T, ExpressionStmt>) {
                evaluate(node->expression); 
            }
            else if constexpr (is_same_v<T, VarStmt>) {
                Literal value = nullptr;
                if (node->initializer.has_value()) {
                    value = evaluate(node->initializer.value());
                }
                environment->define(node->name.lexeme, value);
            }
            else if constexpr (is_same_v<T, Block>) 
            {
                executeBlock(node->statements, std::make_shared<Environment>(environment));
            }
            else if constexpr (is_same_v<T, IfStmt>) 
            {
                Literal conditionResult = evaluate(node->condition);

                if (isTruthy(conditionResult)) 
                {
                    execute(node->thenBranch);
                } 
                else if (node->elseBranch.has_value()) 
                {
                    execute(node->elseBranch.value());
                }
            }
            else if constexpr (is_same_v<T, WhileStmt>) 
            {
                while (isTruthy(evaluate(node->condition))) 
                {
                    execute(node->body);
                }
            }
            else if constexpr (is_same_v<T, FunctionStmt>) 
            {
                std::shared_ptr<LoxCallable> function = std::make_shared<LoxFunction>(node.get(),environment);
                environment->define(node->name.lexeme, function);
            }
            else if constexpr (is_same_v<T, ReturnStmt>) // <-- Fix is right here!
            {
                Literal value = nullptr;
                // If the return statement actually has a value (not just 'return;')
                if (node->value.has_value()) {
                    value = evaluate(node->value.value());
                }
                
                throw ReturnException(value);
            }
        }, stmt);
    }
public:
    void executeBlock(const vector<Stmt>& statements, std::shared_ptr<Environment> newEnvironment) {
        std::shared_ptr<Environment> previous = this->environment;

        try {
            this->environment = newEnvironment;

            for (const Stmt& statement : statements) {
                execute(statement);
            }
        } catch (...) {
            this->environment = previous;
            throw;
        }

        this->environment = previous;
    }

public:
    Literal evaluate(const Expr& expr) 
    {
        return visit([this](const auto& node) -> Literal 
        {
            using T = decay_t<decltype(*node)>;

            if constexpr (is_same_v<T, LiteralExpr>) 
            {
                return node->value;
            } 
            else if constexpr (is_same_v<T, Grouping>) 
            {
                return evaluate(node->expression);
            } 
            else if constexpr (is_same_v<T, Unary>) 
            {
                Literal right = evaluate(node->right);

                switch (node->op.type) 
                {
                    case TokenType::BANG:
                        return !isTruthy(right);
                    case TokenType::MINUS:
                        checkNumberOperand(node->op, right);
                        return -get<double>(right);
                    default:
                        return nullptr;
                }
            }
            else if constexpr (is_same_v<T, Binary>) 
            {
                Literal left = evaluate(node->left);
                Literal right = evaluate(node->right);

                switch (node->op.type) 
                {
                    case TokenType::MINUS:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) - get<double>(right);
                    case TokenType::SLASH:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) / get<double>(right);
                    case TokenType::STAR:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) * get<double>(right);
                    case TokenType::PLUS:
                        if (holds_alternative<double>(left) && holds_alternative<double>(right)) 
                        {
                            return get<double>(left) + get<double>(right);
                        }
                        if (holds_alternative<string>(left) && holds_alternative<string>(right)) 
                        {
                            return get<string>(left) + get<string>(right);
                        }
                        throw RuntimeError(node->op, "Operands must be two numbers or two strings.");
                    case TokenType::GREATER:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) > get<double>(right);
                    case TokenType::GREATER_EQUAL:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) >= get<double>(right);
                    case TokenType::LESS:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) < get<double>(right);
                    case TokenType::LESS_EQUAL:
                        checkNumberOperands(node->op, left, right);
                        return get<double>(left) <= get<double>(right);
                    case TokenType::BANG_EQUAL:
                        return !isEqual(left, right);
                    case TokenType::EQUAL_EQUAL:
                        return isEqual(left, right);
                    default:
                        return nullptr;
                }
            }
            else if constexpr (is_same_v<T, Variable>) 
            {
                // CHANGED: Use the helper instead of environment->get()
                return lookUpVariable(node->name, node.get());
            }
            else if constexpr (is_same_v<T, Assign>) 
            {
                Literal value = evaluate(node->value);
                
                // CHANGED: Use assignAt if resolved, otherwise global assign
                auto it = locals.find(node.get());
                if (it != locals.end()) {
                    int distance = it->second;
                    environment->assignAt(distance, node->name, value);
                } else {
                    globals->assign(node->name, value);
                }
                
                return value;
            }
            else if constexpr (is_same_v<T, Call>) 
            {
                Literal callee = evaluate(node->callee);

                std::vector<Literal> arguments;
                for (const Expr& arg : node->arguments) {
                    arguments.push_back(evaluate(arg));
                }
                
                if (!std::holds_alternative<std::shared_ptr<LoxCallable>>(callee)) {
                    throw RuntimeError(node->paren, "Can only call functions and classes.");
                }

                auto function = std::get<std::shared_ptr<LoxCallable>>(callee);

                if (arguments.size() != function->arity()) {
                    throw RuntimeError(node->paren, 
                        "Expected " + std::to_string(function->arity()) + 
                        " arguments but got " + std::to_string(arguments.size()) + ".");
                }

                return function->call(*this, arguments);
            }
            else if constexpr (is_same_v<T, Logical>) 
            {
                Literal left = evaluate(node->left);

                if (node->op.type == TokenType::OR) {
                    if (isTruthy(left)) return left;
                }
                else { 
                    if (!isTruthy(left)) return left;
                }

                return evaluate(node->right);
            }
        }, expr);
    }
    
    void interpret(const vector<Stmt>& statements) 
    {
        try {
            for (const Stmt& statement : statements) {
                execute(statement);
            }
        } catch (const RuntimeError& error) {
            cerr << error.what() << "\n[line " << error.token.line << "]\n";
        }
    }
};

inline Literal ClockCallable::call(Interpreter& interpreter, std::vector<Literal>& arguments) {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration<double>(now).count(); 
}

inline Literal LoxFunction::call(Interpreter& interpreter, std::vector<Literal>& arguments) {
    auto environment = std::make_shared<Environment>(closure);
    
    for (size_t i = 0; i < declaration->params.size(); ++i) {
        environment->define(declaration->params[i].lexeme, arguments[i]);
    }
    
    try {
        interpreter.executeBlock(declaration->body, environment);
    } 
    catch (ReturnException& returnValue) {
        // We caught the escape hatch! Return the payload.
        return returnValue.value;
    }
    
    // If the function finishes without hitting a return statement, it implicitly returns nil.
    return nullptr; 
}