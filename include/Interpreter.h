#pragma once
#include "Expr.h"
#include "Token.h"
#include "Stmt.h"
#include "RuntimeError.h"
#include "Environment.h"
#include "LoxCallable.h"
#include "LoxInstance.h"
#include <variant>
#include <type_traits>
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <exception>
#include <map>
#include <cstddef>

class Interpreter;

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
    bool isInitializer;

public:
    LoxFunction(FunctionStmt* declaration, std::shared_ptr<Environment> closure, bool isInitializer) 
        : declaration(declaration), closure(std::move(closure)), isInitializer(isInitializer) {}
        
    int arity() override { return declaration->params.size(); }
    
    Literal call(Interpreter& interpreter, std::vector<Literal>& arguments) override;

    std::shared_ptr<LoxFunction> bind(std::shared_ptr<LoxInstance> instance);
    
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
        if (std::holds_alternative<double>(operand)) return;
        throw RuntimeError(op, "Operand must be a number.");
    }

    void checkNumberOperands(const Token& op, const Literal& left, const Literal& right) {
        if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) return;
        throw RuntimeError(op, "Operands must be numbers.");
    }

    bool isTruthy(const Literal& object) 
    {
        if (std::holds_alternative<std::nullptr_t>(object)) return false;
        if (std::holds_alternative<bool>(object)) return std::get<bool>(object);
        return true;
    }
    
    bool isEqual(const Literal& a, const Literal& b) 
    {
        return a == b; 
    }
    
    std::string stringify(const Literal& value) 
    {
        return std::visit([](const auto& val) -> std::string {
            using ValT = std::decay_t<decltype(val)>;
            
            if constexpr (std::is_same_v<ValT, std::nullptr_t>) return "nil";
            else if constexpr (std::is_same_v<ValT, bool>) return val ? "true" : "false";
            else if constexpr (std::is_same_v<ValT, std::string>) return val;
            else if constexpr (std::is_same_v<ValT, double>) 
            {
                std::string text = std::to_string(val);
                text.erase(text.find_last_not_of('0') + 1, std::string::npos);
                if (text.back() == '.') text.pop_back();
                return text;
            }
            else if constexpr (std::is_same_v<ValT, std::shared_ptr<LoxCallable>>) 
            {
                return val->toString();
            }
            else if constexpr (std::is_same_v<ValT, std::shared_ptr<LoxInstance>>) 
            {
                return val->toString();
            }
            return "";
        }, value);
    }

    void execute(const Stmt& stmt) 
    {
        std::visit([this](const auto& node) {
            using T = std::decay_t<decltype(*node)>;

            if constexpr (std::is_same_v<T, PrintStmt>) {
                Literal value = evaluate(node->expression);
                std::cout << stringify(value) << "\n";
            } 
            else if constexpr (std::is_same_v<T, ExpressionStmt>) {
                evaluate(node->expression); 
            }
            else if constexpr (std::is_same_v<T, VarStmt>) {
                Literal value = nullptr;
                if (node->initializer.has_value()) {
                    value = evaluate(node->initializer.value());
                }
                environment->define(node->name.lexeme, value);
            }
            else if constexpr (std::is_same_v<T, Block>) 
            {
                executeBlock(node->statements, std::make_shared<Environment>(environment));
            }
            else if constexpr (std::is_same_v<T, IfStmt>) 
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
            else if constexpr (std::is_same_v<T, WhileStmt>) 
            {
                while (isTruthy(evaluate(node->condition))) 
                {
                    execute(node->body);
                }
            }
            else if constexpr (std::is_same_v<T, FunctionStmt>) 
            {
                std::shared_ptr<LoxCallable> function = std::make_shared<LoxFunction>(node.get(), environment, false);
                environment->define(node->name.lexeme, function);
            }
            else if constexpr (std::is_same_v<T, ClassStmt>) 
            {
                std::shared_ptr<LoxClass> superclass = nullptr;
                if (node->superclass != nullptr) {
                    Literal superclassObj = lookUpVariable(node->superclass->name, node->superclass.get());
                    
                    if (std::holds_alternative<std::shared_ptr<LoxCallable>>(superclassObj)) {
                        auto callable = std::get<std::shared_ptr<LoxCallable>>(superclassObj);
                        superclass = std::dynamic_pointer_cast<LoxClass>(callable);
                    }
                    
                    if (superclass == nullptr) {
                        throw RuntimeError(node->superclass->name, "Superclass must be a class.");
                    }
                }

                std::shared_ptr<Environment> previous = environment;
                if (superclass != nullptr) {
                    environment = std::make_shared<Environment>(environment);
                    environment->define("super", superclass);
                }

                std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods;
                
                for (const auto& method : node->methods) {
                    bool isInitializer = (method->name.lexeme == "init");
                    auto function = std::make_shared<LoxFunction>(method.get(), environment, isInitializer);
                    methods[method->name.lexeme] = function;
                }

                std::shared_ptr<LoxCallable> klass = std::make_shared<LoxClass>(node->name.lexeme, superclass, std::move(methods));
                
                environment = previous;
                
                environment->define(node->name.lexeme, klass);
            }
            else if constexpr (std::is_same_v<T, ReturnStmt>)
            {
                Literal value = nullptr;
                if (node->value.has_value()) {
                    value = evaluate(node->value.value());
                }
                
                throw ReturnException(value);
            }
        }, stmt);
    }
public:
    void executeBlock(const std::vector<Stmt>& statements, std::shared_ptr<Environment> newEnvironment) {
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
        return std::visit([this](const auto& node) -> Literal 
        {
            using T = std::decay_t<decltype(*node)>;

            if constexpr (std::is_same_v<T, LiteralExpr>) 
            {
                return node->value;
            } 
            else if constexpr (std::is_same_v<T, Grouping>) 
            {
                return evaluate(node->expression);
            } 
            else if constexpr (std::is_same_v<T, Unary>) 
            {
                Literal right = evaluate(node->right);

                switch (node->op.type) 
                {
                    case TokenType::BANG:
                        return !isTruthy(right);
                    case TokenType::MINUS:
                        checkNumberOperand(node->op, right);
                        return -std::get<double>(right);
                    default:
                        return nullptr;
                }
            }
            else if constexpr (std::is_same_v<T, Binary>) 
            {
                Literal left = evaluate(node->left);
                Literal right = evaluate(node->right);

                switch (node->op.type) 
                {
                    case TokenType::MINUS:
                        checkNumberOperands(node->op, left, right);
                        return std::get<double>(left) - std::get<double>(right);
                    case TokenType::SLASH:
                        checkNumberOperands(node->op, left, right);
                        return std::get<double>(left) / std::get<double>(right);
                    case TokenType::STAR:
                        checkNumberOperands(node->op, left, right);
                        return std::get<double>(left) * std::get<double>(right);
                    case TokenType::PLUS:
                        if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) 
                        {
                            return std::get<double>(left) + std::get<double>(right);
                        }
                        if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) 
                        {
                            return std::get<std::string>(left) + std::get<std::string>(right);
                        }
                        throw RuntimeError(node->op, "Operands must be two numbers or two strings.");
                    case TokenType::GREATER:
                        checkNumberOperands(node->op, left, right);
                        return std::get<double>(left) > std::get<double>(right);
                    case TokenType::GREATER_EQUAL:
                        checkNumberOperands(node->op, left, right);
                        return std::get<double>(left) >= std::get<double>(right);
                    case TokenType::LESS:
                        checkNumberOperands(node->op, left, right);
                        return std::get<double>(left) < std::get<double>(right);
                    case TokenType::LESS_EQUAL:
                        checkNumberOperands(node->op, left, right);
                        return std::get<double>(left) <= std::get<double>(right);
                    case TokenType::BANG_EQUAL:
                        return !isEqual(left, right);
                    case TokenType::EQUAL_EQUAL:
                        return isEqual(left, right);
                    default:
                        return nullptr;
                }
            }
            else if constexpr (std::is_same_v<T, Variable>) 
            {
                return lookUpVariable(node->name, node.get());
            }
            else if constexpr (std::is_same_v<T, Assign>) 
            {
                Literal value = evaluate(node->value);
                
                auto it = locals.find(node.get());
                if (it != locals.end()) {
                    int distance = it->second;
                    environment->assignAt(distance, node->name, value);
                } else {
                    globals->assign(node->name, value);
                }
                
                return value;
            }
            else if constexpr (std::is_same_v<T, Call>) 
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
            else if constexpr (std::is_same_v<T, Get>) 
            {
                Literal object = evaluate(node->object);
                if (std::holds_alternative<std::shared_ptr<LoxInstance>>(object)) 
                {
                    return std::get<std::shared_ptr<LoxInstance>>(object)->get(node->name);
                }

                throw RuntimeError(node->name, "Only instances have properties.");
            }
            else if constexpr (std::is_same_v<T, Set>) 
            {
                Literal object = evaluate(node->object);

                if (!std::holds_alternative<std::shared_ptr<LoxInstance>>(object)) 
                {
                    throw RuntimeError(node->name, "Only instances have fields.");
                }

                Literal value = evaluate(node->value);
                std::get<std::shared_ptr<LoxInstance>>(object)->set(node->name, value);
                return value;
            }
            else if constexpr (std::is_same_v<T, This>) 
            {
                return lookUpVariable(node->keyword, node.get());
            }
            else if constexpr (std::is_same_v<T, Super>) 
            {
                auto it = locals.find(node.get());
                int distance = it->second;

                Token superToken(TokenType::SUPER, "super", nullptr, 0, 0);
                Literal superclassLit = environment->getAt(distance, superToken);
                auto superclass = std::dynamic_pointer_cast<LoxClass>(std::get<std::shared_ptr<LoxCallable>>(superclassLit));

                Token thisToken(TokenType::THIS, "this", nullptr, 0, 0);
                Literal objectLit = environment->getAt(distance - 1, thisToken);
                auto object = std::get<std::shared_ptr<LoxInstance>>(objectLit);

                std::shared_ptr<LoxFunction> method = superclass->findMethod(node->method.lexeme);
                if (method == nullptr) {
                    throw RuntimeError(node->method, "Undefined property '" + node->method.lexeme + "'.");
                }

                return std::shared_ptr<LoxCallable>(method->bind(object));
            }
            else if constexpr (std::is_same_v<T, Logical>) 
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
    
    void interpret(const std::vector<Stmt>& statements) 
    {
        try {
            for (const Stmt& statement : statements) {
                execute(statement);
            }
        } catch (const RuntimeError& error) {
            std::cerr << error.what() << "\n[line " << error.token.line << "]\n";
        }
    }
};

inline Literal ClockCallable::call(Interpreter& interpreter, std::vector<Literal>& arguments) {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration<double>(now).count(); 
}

inline int LoxClass::arity() {
    std::shared_ptr<LoxFunction> initializer = findMethod("init");
    if (initializer == nullptr) return 0;
    return initializer->arity();
}

inline Literal LoxClass::call(Interpreter& interpreter, std::vector<Literal>& arguments) {
    auto instance = std::make_shared<LoxInstance>(shared_from_this());
    
    std::shared_ptr<LoxFunction> initializer = findMethod("init");
    if (initializer != nullptr) {
        initializer->bind(instance)->call(interpreter, arguments);
    }
    
    return instance;
}

inline std::shared_ptr<LoxFunction> LoxFunction::bind(std::shared_ptr<LoxInstance> instance) {
    auto environment = std::make_shared<Environment>(closure);
    environment->define("this", instance);
    return std::make_shared<LoxFunction>(declaration, environment, isInitializer);
}

inline Literal LoxInstance::get(const Token& name) {
    auto it = fields.find(name.lexeme);
    if (it != fields.end()) {
        return it->second;
    }

    std::shared_ptr<LoxFunction> method = klass->findMethod(name.lexeme);
    if (method != nullptr) {
        return std::shared_ptr<LoxCallable>(method->bind(shared_from_this())); 
    }

    throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

inline void LoxInstance::set(const Token& name, Literal value) {
    fields[name.lexeme] = std::move(value);
}

inline Literal LoxFunction::call(Interpreter& interpreter, std::vector<Literal>& arguments) {
    auto environment = std::make_shared<Environment>(closure);
    
    for (std::size_t i = 0; i < declaration->params.size(); ++i) {
        environment->define(declaration->params[i].lexeme, arguments[i]);
    }
    
    try {
        interpreter.executeBlock(declaration->body, environment);
    } 
    catch (ReturnException& returnValue) {
        if (isInitializer) {
            Token thisToken(TokenType::THIS, "this", nullptr, 0, 0); 
            return closure->getAt(0, thisToken);
        }
        return returnValue.value;
    }
    
    if (isInitializer) {
        Token thisToken(TokenType::THIS, "this", nullptr, 0, 0); 
        return closure->getAt(0, thisToken);
    }
    
    return nullptr; 
}