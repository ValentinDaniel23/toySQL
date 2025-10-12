#include <gtest/gtest.h>
#include "../OS/specificOS.h"
#include "../storage/ds.h"

TEST(StorageTest, CreateTestDirectory) {
    bool ret = true;

    std :: string path = ROOT;
    path += "testBtree";

    EXPECT_EQ(os::createDir(path), true);
}

TEST(StorageTest, tupleSerialization) {
    Row row{10, 6, "Hello!"};

    std :: array<char, ROW_SIZE> buffer = row.serialize();

    Row row1 = Row::deserialize(buffer);

    EXPECT_EQ(row.id, row1.id);
    EXPECT_EQ(row1.text_size, row.text_size);
    EXPECT_EQ(row.text, row1.text);
}

TEST(StorageTest, basicLeafTest) {
    std :: string path = ROOT;
    path += "testBtree";
    path += OS_SEP;
    path += "leaf.txt";

    if (os :: fileExists(path)) {
        os :: removeFile(path);
    }
    std :: cout << path << '\n';

    Table table{path.c_str(), 4};

    Cursor cursor1 = table.make_cursor();
    Cursor cursor2 = table.make_cursor(0, 0);
    Cursor cursor3 = table.make_cursor(0, 1);

    Row *row1 = new Row{10, 6, "Hello!"};
    Row *row2 = new Row{8, 16, "Michael Jackson!"};
    Row *row3 = new Row{18, 3, "Lol"};

    table.leaf_node_insert(cursor1, row1->id, row1);
    table.leaf_node_insert(cursor2, row2->id, row2);
    table.leaf_node_insert(cursor3, row3->id, row3);

    table.pager.flush_all();

    delete row1;
    delete row2;
    delete row3;

    Cursor cursor = table.make_cursor();
    Row *row = nullptr;

    ASSERT_EQ(cursor.end_of_table, false);
    row = cursor.value();
    ASSERT_EQ(row -> id, 8);
    ASSERT_EQ(row -> text_size, 16);
    ASSERT_STREQ(row -> text.data(), "Michael Jackson!");

    cursor.advance();

    ASSERT_EQ(cursor.end_of_table, false);
    row = cursor.value();
    ASSERT_EQ(row -> id, 18);
    ASSERT_EQ(row -> text_size, 3);
    ASSERT_STREQ(row -> text.data(), "Lol");

    cursor.advance();

    ASSERT_EQ(cursor.end_of_table, false);
    row = cursor.value();
    ASSERT_EQ(row -> id, 10);
    ASSERT_EQ(row -> text_size, 6);
    ASSERT_STREQ(row -> text.data(), "Hello!");

    cursor.advance();

    ASSERT_EQ(cursor.end_of_table, true);
}

TEST(StorageTest, basicSortedLeafTest) {
    std :: string path = ROOT;
    path += "testBtree";
    path += OS_SEP;
    path += "sortedleaf.txt";

    if (os :: fileExists(path)) {
        os :: removeFile(path);
    }

    Table table{path.c_str(), 3};
    init_leaf_node(table.pager.get_page(0)->data.data());

    Row *row1 = new Row{10, 6, "Hello!"};
    Row *row2 = new Row{8, 16, "Michael Jackson!"};
    Row *row3 = new Row{18, 3, "Lol"};
    Row* row4 = new Row{5, 12, "Good morning"};
    Row* row5 = new Row{12, 10, "OpenAI GPT"};
    Row* row6 = new Row{20, 9, "C++ rocks"};

    Cursor cursor1 = table.table_find(10);
    table.leaf_node_insert(cursor1, row1->id, row1);

    Cursor cursor2 = table.table_find(8);
    table.leaf_node_insert(cursor2, row2->id, row2);

    Cursor cursor3 = table.table_find(18);
    table.leaf_node_insert(cursor3, row3->id, row3);

    Cursor cursor4 = table.table_find(5);
    table.leaf_node_insert(cursor4, row4->id, row4);

    Cursor cursor5 = table.table_find(12);
    table.leaf_node_insert(cursor5, row5->id, row5);

    Cursor cursor6 = table.table_find(20);
    table.leaf_node_insert(cursor6, row6->id, row6);

    table.pager.flush_all();

    delete row1;
    delete row2;
    delete row3;
    delete row4;
    delete row5;
    delete row6;

    EXPECT_EQ(1, 1);

    Cursor cursor = table.make_cursor();
    Row *row = nullptr;

    ASSERT_EQ(cursor.end_of_table, false);
    row = cursor.value();
    ASSERT_EQ(row -> id, 5);
    ASSERT_EQ(row -> text_size, 12);
    ASSERT_STREQ(row -> text.data(), "Good morning");

    cursor.advance();

    ASSERT_EQ(cursor.end_of_table, false);
    row = cursor.value();
    ASSERT_EQ(row -> id, 8);
    ASSERT_EQ(row -> text_size, 16);
    ASSERT_STREQ(row -> text.data(), "Michael Jackson!");

    cursor.advance();

    ASSERT_EQ(cursor.end_of_table, false);
    row = cursor.value();
    ASSERT_EQ(row -> id, 10);
    ASSERT_EQ(row -> text_size, 6);
    ASSERT_STREQ(row -> text.data(), "Hello!");

    cursor.advance();

    ASSERT_EQ(cursor.end_of_table, false);
    row = cursor.value();
    ASSERT_EQ(row -> id, 12);
    ASSERT_EQ(row -> text_size, 10);
    ASSERT_STREQ(row -> text.data(), "OpenAI GPT");

    cursor.advance();

    ASSERT_EQ(cursor.end_of_table, false);
    row = cursor.value();
    ASSERT_EQ(row -> id, 18);
    ASSERT_EQ(row -> text_size, 3);
    ASSERT_STREQ(row -> text.data(), "Lol");

    cursor.advance();

    ASSERT_EQ(cursor.end_of_table, false);
    row = cursor.value();
    ASSERT_EQ(row -> id, 20);
    ASSERT_EQ(row -> text_size, 9);
    ASSERT_STREQ(row -> text.data(), "C++ rocks");

    cursor.advance();

    ASSERT_EQ(cursor.end_of_table, true);
}