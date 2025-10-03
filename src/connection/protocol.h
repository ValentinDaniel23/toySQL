#ifndef TOYSQL_PROTOCOL_H
#define TOYSQL_PROTOCOL_H

#include <iostream>
#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

// CREATE TABLE users (id INT, name TEXT);
// INSERT INTO users VALUES (1, 'Alice');    -- Create
// SELECT * FROM users;                       -- Read
// UPDATE users SET name='Bob' WHERE id=1;   -- Update
// DELETE FROM users WHERE id=1;             -- Delete

// CREATE TABLE ... (...);          -- 1
// INSERT INTO ... VALUES (...);    -- 2
// SELECT ... FROM ... WHERE ...;   -- 3
// UPDATE ... SET ... WHERE ...;    -- 4
// DELETE FROM ... WHERE ...;       -- 5

enum class ToySQLTokenType  {
    Number,
    Word,
    Literal,
    OpenBracket,
    CloseBracket,
    Any,
    Comma,
    Equal,
    END,
    Invalid,
};

struct Token {
    ToySQLTokenType  type;
    std::string value;
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

    json serialize();

private:
    json Create();
    json Insert();
    json Select();
    json Update();
    json Delete();

    bool checkWords(std :: string s1, std :: string s2) {
        if (s1.size() != s2.size())
            return false;

        for (int i = 0; i < static_cast<int>(s1.size()); i++) {
            if (!std::isalpha(static_cast<unsigned char>(s1[i])))
                return false;

            if (!std::isalpha(static_cast<unsigned char>(s2[i])))
                return false;

            if (static_cast<unsigned char>(toupper(s1[i])) != static_cast<unsigned char>(toupper(s2[i])))
                return false;
        }

        return true;
    }

    Lexer& lexer;
    Token currentToken;
};


#endif //TOYSQL_PROTOCOL_H