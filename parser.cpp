
#include "parser.h"

#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <string>

Parser::Parser(const PluginLoader& plugins)
    : plugins_(plugins) {
}

void Parser::skipSpaces() {
    while (pos_ < text_.size() &&
        (text_[pos_] == ' ' ||
            text_[pos_] == '\t' ||
            text_[pos_] == '\r' ||
            text_[pos_] == '\n')) {
        ++pos_;
    }
}

bool Parser::consume(char ch) {
    skipSpaces();

    if (pos_ < text_.size() && text_[pos_] == ch) {
        ++pos_;
        return true;
    }

    return false;
}

[[noreturn]] void Parser::syntaxError(
    const std::string& message
) {
    throw std::runtime_error(
        "Syntax error at position " +
        std::to_string(pos_) + ": " + message
    );
}

double Parser::evaluate(const std::string& expression) {
    text_ = expression;
    pos_ = 0;

    double result = parseExpression();

    skipSpaces();

    if (pos_ != text_.size()) {
        syntaxError("unexpected character");
    }

    if (!std::isfinite(result)) {
        throw std::runtime_error(
            "Result is not a finite number"
        );
    }

    return result;
}

// expression = term { ('+' | '-') term }
double Parser::parseExpression() {
    double value = parseTerm();

    while (true) {
        if (consume('+')) {
            value += parseTerm();
        }
        else if (consume('-')) {
            value -= parseTerm();
        }
        else {
            break;
        }
    }

    return value;
}

// term = unary { ('*' | '/') unary }
double Parser::parseTerm() {
    double value = parseUnary();

    while (true) {
        if (consume('*')) {
            value *= parseUnary();
        }
        else if (consume('/')) {
            double divisor = parseUnary();

            if (divisor == 0.0) {
                throw std::runtime_error(
                    "Division by zero"
                );
            }

            value /= divisor;
        }
        else {
            break;
        }
    }

    return value;
}

// unary = ('+' | '-') unary | power
double Parser::parseUnary() {
    if (consume('+')) {
        return parseUnary();
    }

    if (consume('-')) {
        return -parseUnary();
    }

    return parsePower();
}

// power = primary [ '^' unary ]
// This makes exponentiation right-associative.
double Parser::parsePower() {
    double base = parsePrimary();

    if (consume('^')) {
        double exponent = parseUnary();
        double result = std::pow(base, exponent);

        if (!std::isfinite(result)) {
            throw std::runtime_error(
                "Invalid or non-finite power result"
            );
        }

        return result;
    }

    return base;
}

// primary = number
//         | '(' expression ')'
//         | identifier '(' expression ')'
double Parser::parsePrimary() {
    if (consume('(')) {
        double value = parseExpression();

        if (!consume(')')) {
            syntaxError("expected ')'");
        }

        return value;
    }

    skipSpaces();

    if (pos_ >= text_.size()) {
        syntaxError("expected a number or function");
    }

    char ch = text_[pos_];

    if ((ch >= '0' && ch <= '9') || ch == '.') {
        return parseNumber();
    }

    if ((ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
        ch == '_') {
        std::string name = parseIdentifier();

        if (!consume('(')) {
            syntaxError("expected '(' after function name");
        }

        double argument = parseExpression();

        if (!consume(')')) {
            syntaxError("expected ')' after function argument");
        }

        return plugins_.call(name, argument);
    }

    syntaxError("unexpected character");
}

std::string Parser::parseIdentifier() {
    skipSpaces();

    std::size_t start = pos_;

    while (pos_ < text_.size()) {
        char ch = text_[pos_];

        if ((ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '_') {
            ++pos_;
        }
        else {
            break;
        }
    }

    if (start == pos_) {
        syntaxError("expected function name");
    }

    return text_.substr(start, pos_ - start);
}

double Parser::parseNumber() {
    skipSpaces();

    const char* start = text_.c_str() + pos_;
    char* end = nullptr;

    double value = std::strtod(start, &end);

    if (end == start) {
        syntaxError("invalid number");
    }

    pos_ += static_cast<std::size_t>(end - start);

    if (!std::isfinite(value)) {
        throw std::runtime_error(
            "Number must be finite"
        );
    }

    return value;
}
