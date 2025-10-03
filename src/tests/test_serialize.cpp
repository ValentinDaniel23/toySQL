#include <gtest/gtest.h>
#include "../connection/protocol.h"

TEST(SerializeTest, Create) {
    {
        Lexer l(" cREAte \r\r\r\n taBLE \n\n users ( id  InT  ,  name TeXt   , age INT) ");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op":"create",
            "table_name":"users",
            "columns":[["id", "INT"], ["name", "TEXT"], ["age", "INT"]]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("CREATE TABLE products ( productID INT )");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op":"create",
            "table_name":"products",
            "columns":[["productID", "INT"]]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("  create\tTABLE orders \n( order_id INT , customer_id INT , total FLOAT ) ");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({})");

        EXPECT_EQ(obj, objtest);
    }
}

TEST(SerializeTest, Insert) {
    {
        Lexer l("INSERT INTO users VALUES (1)");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op":"insert",
            "table_name":"users",
            "values":["1"]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("INsERT IntO users VALUES (1, \"Alice\", 30)");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op":"insert",
            "table_name":"users",
            "values":["1", "Alice", "30"]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("  INSERT  \n INtO users \t VALUES ( 1 , \"Bob\" , 42.5 )  ");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({})");

        EXPECT_EQ(obj, objtest);
    }
}

TEST(SerializeTest, Select) {
    {
        Lexer l(" selECT \n\n\r \r * \r\r from \n\r\r users  ");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op":"select",
            "table_name":"users",
            "columns":["*"],
            "where":[]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("select id, name from users where id=1");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op":"select",
            "table_name":"users",
            "columns":["id", "name"],
            "where":[["id", "1"]]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("select from users where id=1");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({})");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("select id, name from users where id=1 ,name=\"Alice\"");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op": "select",
            "table_name": "users",
            "columns": ["id", "name"],
            "where": [["id", "1"], ["name", "Alice"]]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("select id,name,age from people");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op": "select",
            "table_name": "people",
            "columns": ["id", "name", "age"],
            "where": []
        })");

        EXPECT_EQ(obj, objtest);
    }
}

TEST(SerializeTest, Update) {
    {
        Lexer l("UPDATE users SET name=\"Bob\" WHERE id=1");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op":"update",
            "table_name":"users",
            "values":[["name","Bob"]],
            "where":[["id","1"]]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("UPDATE users SET name=\"Bob\", age=30 WHERE id=1");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op":"update",
            "table_name":"users",
            "values":[["name","Bob"],["age","30"]],
            "where":[["id","1"]]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("UPDATE users SET name=\"Charlie\", age=25 WHERE id=2, name=\"Bob\"");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op":"update",
            "table_name":"users",
            "values":[["name","Charlie"],["age","25"]],
            "where":[["id","2"],["name","Bob"]]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("UPDATE users SET age=40");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op":"update",
            "table_name":"users",
            "values":[["age","40"]],
            "where":[]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("UPDATE users name='Bob' WHERE id=1;");
        Parser p(l);

        json obj = p.serialize();
        EXPECT_EQ(obj, json::object());
    }

    {
        Lexer l("UPDATE users SET name= WHERE id=1;");
        Parser p(l);

        json obj = p.serialize();
        EXPECT_EQ(obj, json::object());
    }
}

TEST(SerializeTest, Delete) {
    {
        Lexer l("DELETE FROM users WHERE id=1");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op": "delete",
            "table_name": "users",
            "where": [["id", "1"]]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("DELETE FROM users");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op": "delete",
            "table_name": "users",
            "where": []
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("DELETE FROM users WHERE id=1,name=\"Alice\"");
        Parser p(l);

        json obj = p.serialize();
        json objtest = json::parse(R"({
            "op": "delete",
            "table_name": "users",
            "where": [["id","1"],["name","Alice"]]
        })");

        EXPECT_EQ(obj, objtest);
    }

    {
        Lexer l("DELETE users WHERE id=1");
        Parser p(l);

        json obj = p.serialize();
        EXPECT_EQ(obj, json::object());
    }

    {
        Lexer l("DELETE FROM users WHERE id=;");
        Parser p(l);

        json obj = p.serialize();
        EXPECT_EQ(obj, json::object());
    }
}