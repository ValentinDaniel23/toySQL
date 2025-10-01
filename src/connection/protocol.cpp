#include "protocol.h"

Token Lexer::nextToken() {
    while (pos < input.length() && std::isspace(input[pos])) {
        ++pos;
    }

    if (pos >= input.length()) {
        return {ToySQLTokenType::END, ""};
    }

    char current = input[pos];
    if (std :: isdigit(current)) {
        return number();
    } else if (std :: isalpha(current)) {
        return word();
    } else if (current == '(') {
        ++pos;
        return {ToySQLTokenType::OpenBracket, "("};
    } else if (current == ')') {
        ++pos;
        return {ToySQLTokenType::CloseBracket, ")"};
    } else if (current == '*') {
        ++pos;
        return {ToySQLTokenType::Any, "*"};
    } else if (current == ',') {
        ++pos;
        return {ToySQLTokenType::Comma, ","};
    } else if (current == '=') {
        ++pos;
        return {ToySQLTokenType::Assign, "\""};
    } else if (current == '\"') {
        return literal();
    }

    ++pos;
    return {ToySQLTokenType::Invalid, std::string(1, current)};
}

Token Lexer::number() {
    size_t start = pos;
    while (pos < input.length() && std::isdigit(input[pos])) {
        ++pos;
    }
    return {ToySQLTokenType::Number, input.substr(start, pos - start)};
}

Token Lexer::word() {
    size_t start = pos;
    while (pos < input.length() && std::isalpha(input[pos])) {
        ++pos;
    }
    while (pos < input.length() && std::isdigit(input[pos])) {
        ++pos;
    }

    return {ToySQLTokenType::Word, input.substr(start, pos - start)};
}

Token Lexer::literal() {
    pos ++;
    size_t start = pos;

    while (pos < input.length() && input[pos] != '"') {
        ++pos;
    }

    if (pos == input.length()) {
        return {ToySQLTokenType::Invalid, ""};
    }

    pos ++;

    return {ToySQLTokenType::Literal, input.substr(start, pos - start)};
}

int Parser::parse() {
    while (currentToken.typee != ToySQLTokenType::END) {
        std :: cout << currentToken.value << ";";
        currentToken = lexer.nextToken();
    }
    std :: cout << std :: endl;
    return 0;
}