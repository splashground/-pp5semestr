
#pragma once

#include "plugin_loader.h"

#include <cstddef>
#include <string>

class Parser {
public:
    explicit Parser(const PluginLoader& plugins);

    double evaluate(const std::string& expression);

private:
    const PluginLoader& plugins_;
    std::string text_;
    std::size_t pos_ = 0;

    void skipSpaces();
    bool consume(char ch);

    double parseExpression();
    double parseTerm();
    double parseUnary();
    double parsePower();
    double parsePrimary();

    std::string parseIdentifier();
    double parseNumber();

    [[noreturn]] void syntaxError(
        const std::string& message
    );
};
