#include "protocol.h"

Token Lexer::nextToken() {
    while (pos < input.length() && std::isspace(input[pos])) {
        ++pos;
    }

    if (pos >= input.length()) {
        return {ToySQLTokenType::END, ""};
    }

    char current = input[pos];
    if (std::isdigit(static_cast<unsigned char>(current))) {
        return number();
    } else if (std::isalpha(static_cast<unsigned char>(current))) {
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
        return {ToySQLTokenType::Equal, "\""};
    } else if (current == '\"') {
        return literal();
    }

    ++pos;
    return {ToySQLTokenType::Invalid, std::string(1, current)};
}

Token Lexer::number() {
    size_t start = pos;
    while (pos < input.length() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
        ++pos;
    }
    return {ToySQLTokenType::Number, input.substr(start, pos - start)};
}

Token Lexer::word() {
    size_t start = pos;
    while (pos < input.length() && std::isalpha(static_cast<unsigned char>(input[pos]))) {
        ++pos;
    }
    while (pos < input.length() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
        ++pos;
    }

    return {ToySQLTokenType::Word, input.substr(start, pos - start)};
}

Token Lexer::literal() {
    pos ++;
    size_t start = pos;

    while (pos < input.length() && input[pos] != '\"') {
        ++pos;
    }

    if (pos == input.length()) {
        return {ToySQLTokenType::Invalid, ""};
    }

    pos ++;

    return {ToySQLTokenType::Literal, input.substr(start, pos - 1 - start)};
}

json Parser::serialize() {
    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    if (currentToken.value.size() != 6)
        return json::object();;

    std::unordered_map<std::string, std::function<json()>> commands = {
        {"CREATE", [this]() { return this->Create(); }},
        {"INSERT", [this]() { return this->Insert(); }},
        {"SELECT", [this]() { return this->Select(); }},
        {"UPDATE", [this]() { return this->Update(); }},
        {"DELETE", [this]() { return this->Delete(); }}
    };

    for (const auto& obj : commands) {
        if (checkWords(currentToken.value, obj.first)) {
            return obj.second();
        }
    }

    return json::object();;
}

json Parser::Create() {
    json obj{};

    obj["op"] = "create";

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    if (!checkWords(currentToken.value, "TABLE"))
        return json::object();;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    obj["table_name"] = currentToken.value;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::OpenBracket)
        return json::object();;

    obj["columns"] = json::array();

    while (1) {
        std :: string colName, colType;

        currentToken = lexer.nextToken();
        if (currentToken.type != ToySQLTokenType::Word)
            return json::object();;
        colName = currentToken.value;

        currentToken = lexer.nextToken();
        if (currentToken.type != ToySQLTokenType::Word)
            return json::object();;

        colType = currentToken.value;

        if (checkWords(colType, "INT"))
            colType = "INT";
        else if (checkWords(colType, "TEXT"))
            colType = "TEXT";
        else return json::object();

        obj["columns"].push_back({colName, colType});

        currentToken = lexer.nextToken();
        if (currentToken.type == ToySQLTokenType::CloseBracket)
            break;
        if (currentToken.type == ToySQLTokenType::Comma)
            continue;

        return json::object();;
    }

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::END)
        return json::object();;

    return obj;
}

json Parser::Insert() {
    json obj{};

    obj["op"] = "insert";

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    if (!checkWords(currentToken.value, "INTO"))
        return json::object();;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    obj["table_name"] = currentToken.value;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    if (!checkWords(currentToken.value, "VALUES"))
        return json::object();;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::OpenBracket)
        return json::object();;

    obj["values"] = json::array();

    while (1) {
        std :: string colValue;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Number && currentToken.type != ToySQLTokenType::Literal)
            return json::object();;

        colValue = currentToken.value;

        obj["values"].push_back(colValue);

        currentToken = lexer.nextToken();
        if (currentToken.type == ToySQLTokenType::CloseBracket)
            break;
        if (currentToken.type == ToySQLTokenType::Comma)
            continue;

        return json::object();;
    }

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::END)
        return json::object();;

    return obj;
}

json Parser::Select() {
    json obj{};

    obj["op"] = "select";
    obj["where"] = json::array();

    currentToken = lexer.nextToken();

    obj["columns"] = json::array();

    if (currentToken.type == ToySQLTokenType::Word) {
        obj["columns"].push_back(currentToken.value);

        while (true) {
            currentToken = lexer.nextToken();

            if (currentToken.type != ToySQLTokenType::Comma)
                break;

            currentToken = lexer.nextToken();

            if (currentToken.type != ToySQLTokenType::Word)
                return json::object();;

            obj["columns"].push_back(currentToken.value);
        }
    } else if (currentToken.type == ToySQLTokenType::Any) {
        obj["columns"].push_back("*");

        currentToken = lexer.nextToken();
    } else {
        return json::object();;
    }

    if (!checkWords(currentToken.value, "FROM"))
        return json::object();;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    obj["table_name"] = currentToken.value;

    currentToken = lexer.nextToken();

    if (currentToken.type == ToySQLTokenType::END)
        return obj;

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    if (!checkWords(currentToken.value, "WHERE"))
        return json::object();;

    while (1) {
        std :: string key, value;
        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Word)
            return json::object();;

        key = currentToken.value;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Equal)
            return json::object();;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Number && currentToken.type != ToySQLTokenType::Literal)
            return json::object();;

        value = currentToken.value;

        obj["where"].push_back({key, value});

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Comma)
            break;
    }

    if (currentToken.type != ToySQLTokenType::END)
        return json::object();;

    return obj;
}

json Parser::Update() {
    json obj{};

    obj["op"] = "update";
    obj["where"] = json::array();

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    obj["table_name"] = currentToken.value;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    if (!checkWords(currentToken.value, "SET"))
        return json::object();;

    obj["values"] = json::array();

    while (1) {
        std :: string key, value;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Word)
            return json::object();;

        key = currentToken.value;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Equal)
            return json::object();;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Number && currentToken.type != ToySQLTokenType::Literal)
            return json::object();;

        value = currentToken.value;

        obj["values"].push_back({key, value});

        currentToken = lexer.nextToken();

        if (currentToken.type == ToySQLTokenType::Comma)
            continue;

        break;
    }

    if (currentToken.type == ToySQLTokenType::END)
        return obj;

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    if (!checkWords(currentToken.value, "WHERE"))
        return json::object();;

    while (1) {
        std :: string key, value;
        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Word)
            return json::object();;

        key = currentToken.value;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Equal)
            return json::object();;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Number && currentToken.type != ToySQLTokenType::Literal)
            return json::object();;

        value = currentToken.value;

        obj["where"].push_back({key, value});

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Comma)
            break;
    }

    if (currentToken.type != ToySQLTokenType::END)
        return json::object();;

    return obj;
}

json Parser::Delete() {
    json obj{};

    obj["op"] = "delete";
    obj["where"] = json::array();

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    if (!checkWords(currentToken.value, "FROM"))
        return json::object();;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    obj["table_name"] = currentToken.value;

    currentToken = lexer.nextToken();

    if (currentToken.type == ToySQLTokenType::END)
        return obj;

    if (currentToken.type != ToySQLTokenType::Word)
        return json::object();;

    if (!checkWords(currentToken.value, "WHERE"))
        return json::object();;

    while (1) {
        std :: string key, value;
        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Word)
            return json::object();;

        key = currentToken.value;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Equal)
            return json::object();;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Number && currentToken.type != ToySQLTokenType::Literal)
            return json::object();;

        value = currentToken.value;

        obj["where"].push_back({key, value});

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Comma)
            break;
    }

    if (currentToken.type != ToySQLTokenType::END)
        return json::object();;

    return obj;
}