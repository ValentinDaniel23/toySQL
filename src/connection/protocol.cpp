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
        return json{};

    if (currentToken.value.size() != 6)
        return json{};

    std::unordered_map<std::string, std::function<json()>> commands = {
        {"CREATE", [this]() { return this->Create(); }},
        {"INSERT", [this]() { return this->Insert(); }},
        {"SELECT", [this]() { return this->Select(); }},
        {"UPDATE", [this]() { return this->Update(); }},
        {"DELETE", [this]() { return this->Delete(); }}
    };

    for (const auto& obj : commands) {
        if (checkWords(currentToken.value, obj.first))
            return obj.second();
    }

    // while (currentToken.type != ToySQLTokenType::END) {
    //     std :: cout << currentToken.value << ";";
    //     currentToken = lexer.nextToken();
    // }

    return json{};
}

json Parser::Create() {
    json obj{};

    obj["op"] = "create";

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    if (!checkWords(currentToken.value, "TABLE"))
        return json{};

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    obj["table_name"] = currentToken.value;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::OpenBracket)
        return json{};

    obj["columns"] = json::array();

    while (1) {
        std :: string colName, colType;

        currentToken = lexer.nextToken();
        if (currentToken.type != ToySQLTokenType::Word)
            return json{};
        colName = currentToken.value;

        currentToken = lexer.nextToken();
        if (currentToken.type != ToySQLTokenType::Word)
            return json{};
        colType = currentToken.value;

        obj["columns"].push_back({colName, colType});

        if (!checkWords(colType, "INT") && !checkWords(colType, "TEXT"))
            return json{};

        currentToken = lexer.nextToken();
        if (currentToken.type == ToySQLTokenType::CloseBracket)
            break;
        if (currentToken.type == ToySQLTokenType::Comma)
            continue;

        return json{};
    }

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::END)
        return json{};

    return obj;
}

json Parser::Insert() {
    json obj{};

    obj["op"] = "insert";

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    if (!checkWords(currentToken.value, "INTO"))
        return json{};

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    obj["table_name"] = currentToken.value;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    if (!checkWords(currentToken.value, "VALUES"))
        return json{};

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::OpenBracket)
        return json{};

    obj["values"] = json::array();

    while (1) {
        std :: string colValue;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Number && currentToken.type != ToySQLTokenType::Literal)
            return json{};

        colValue = currentToken.value;

        obj["values"].push_back(colValue);

        currentToken = lexer.nextToken();
        if (currentToken.type == ToySQLTokenType::CloseBracket)
            break;
        if (currentToken.type == ToySQLTokenType::Comma)
            continue;

        return json{};
    }

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::END)
        return json{};

    return obj;
}

json Parser::Select() {
    json obj{};

    obj["op"] = "select";

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
                return json{};

            obj["columns"].push_back(currentToken.value);
        }
    } else if (currentToken.type == ToySQLTokenType::Any) {
        obj["columns"].push_back("*");

        currentToken = lexer.nextToken();
    } else {
        return json{};
    }

    if (!checkWords(currentToken.value, "FROM"))
        return json{};

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    obj["table_name"] = currentToken.value;

    currentToken = lexer.nextToken();

    if (currentToken.type == ToySQLTokenType::END)
        return obj;

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    if (!checkWords(currentToken.value, "WHERE"))
        return json{};

    obj["where"] = json::array();

    while (1) {
        std :: string key, value;
        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Word)
            return json{};

        key = currentToken.value;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Equal)
            return json{};

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Number && currentToken.type != ToySQLTokenType::Literal)
            return json{};

        value = currentToken.value;

        obj["where"].push_back({key, value});

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Comma)
            break;
    }

    if (currentToken.type != ToySQLTokenType::END)
        return json{};

    return obj;
}

json Parser::Update() {
    json obj{};

    obj["op"] = "update";

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    obj["table_name"] = currentToken.value;

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    if (!checkWords(currentToken.value, "SET"))
        return json{};

    obj["values"] = json::array();

    while (1) {
        std :: string key, value;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Word)
            return json{};

        key = currentToken.value;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Equal)
            return json{};

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Number && currentToken.type != ToySQLTokenType::Literal)
            return json{};

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
        return json{};

    if (!checkWords(currentToken.value, "WHERE"))
        return json{};

    obj["where"] = json::array();

    while (1) {
        std :: string key, value;
        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Word)
            return json{};

        key = currentToken.value;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Equal)
            return json{};

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Number && currentToken.type != ToySQLTokenType::Literal)
            return json{};

        value = currentToken.value;

        obj["where"].push_back({key, value});

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Comma)
            break;
    }

    if (currentToken.type != ToySQLTokenType::END)
        return json{};

    return obj;
}

json Parser::Delete() {
    json obj{};

    obj["op"] = "delete";

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    if (!checkWords(currentToken.value, "FROM"))
        return json{};

    currentToken = lexer.nextToken();

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    obj["table_name"] = currentToken.value;

    currentToken = lexer.nextToken();

    if (currentToken.type == ToySQLTokenType::END)
        return obj;

    if (currentToken.type != ToySQLTokenType::Word)
        return json{};

    if (!checkWords(currentToken.value, "WHERE"))
        return json{};

    obj["where"] = json::array();

    while (1) {
        std :: string key, value;
        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Word)
            return json{};

        key = currentToken.value;

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Equal)
            return json{};

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Number && currentToken.type != ToySQLTokenType::Literal)
            return json{};

        value = currentToken.value;

        obj["where"].push_back({key, value});

        currentToken = lexer.nextToken();

        if (currentToken.type != ToySQLTokenType::Comma)
            break;
    }

    if (currentToken.type != ToySQLTokenType::END)
        return json{};

    return obj;
}