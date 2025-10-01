#ifndef TOYSQL_PROTOCOL_H
#define TOYSQL_PROTOCOL_H

#include <iostream>
#include <string>
#include <stdexcept>

// CREATE TABLE users (id INT, name TEXT);
// INSERT INTO users VALUES (1, 'Alice');    -- Create
// SELECT * FROM users;                       -- Read
// UPDATE users SET name='Bob' WHERE id=1;   -- Update
// DELETE FROM users WHERE id=1;             -- Delete

// CREATE TABLE ... (...);
// INSERT INTO ... VALUES (...);
// SELECT ... FROM ... WHERE ...;
// UPDATE ... SET ... WHERE ...;
// DELETE FROM ... WHERE ...;

enum class ToySQLTokenType  {
    Number,
    Word,
    Literal,
    OpenBracket,
    CloseBracket,
    Any,
    Comma,
    Assign,
    END,
    Invalid,
};

struct Token {
    ToySQLTokenType  typee;
    std::string value;
};

class protocol {
    const std :: string& input_buffer;

public:
    explicit protocol(const std ::string& input_buffer) : input_buffer(input_buffer) {}

    std :: string tokenize();
};

class Lexer {
public:
    explicit Lexer(const std::string& input) : input(input), pos(0) {}

    Token nextToken();

private:
    std::string input;
    size_t pos;

    Token number();
    Token word();
    Token literal();
};

class Parser {
public:
    explicit Parser(Lexer& lexer) : lexer(lexer), currentToken(lexer.nextToken()) {}

    int parse();

private:
    Lexer& lexer;
    Token currentToken;

    void eval(ToySQLTokenType  type);
};

#endif //TOYSQL_PROTOCOL_H