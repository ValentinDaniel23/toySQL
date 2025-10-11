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

TEST(StorageTest, pager) {
    std :: string path = ROOT;
    path += "testBtree";
    path += OS_SEP;
    path += "testBtree.txt";

    std :: cout << path << '\n';

    Table table{path.c_str(), 4};
    Cursor cursor = table.make_cursor();

    while (!cursor.end_of_table) {
        Row *row = cursor.value();
        std :: cout << row->id << ' ' << row->text.data() << '\n';

        cursor.advance();
    }

    // Cursor cursor1 = table.make_cursor();
    // Cursor cursor2 = table.make_cursor(0, 0);
    // Cursor cursor3 = table.make_cursor(0, 1);
    //
    // Row *row1 = new Row{10, 6, "Hello!"};
    // Row *row2 = new Row{8, 16, "Michael Jackson!"};
    // Row *row3 = new Row{18, 3, "Lol"};
    //
    // table.leaf_node_insert(cursor1, row1->id, row1);
    // table.leaf_node_insert(cursor1, row2->id, row2);
    // table.leaf_node_insert(cursor1, row3->id, row3);
    //
    // table.pager.flush_all();
    //
    // delete row1;
    // delete row2;
    // delete row3;

    {
        // Pager pager(path.c_str(), 10);
        //
        // void* ce0 = pager.get_page(0);
        // void* ce1 = pager.get_page(1);
        // void* ce2 = pager.get_page(2);
        // void* ce3 = pager.get_page(3);
        //
        // Row row{10, 6, "Hello!"};
        //
        // std :: array<char, ROW_SIZE> buffer = row.serialize();
        // std::memcpy(ce0, buffer.data(), buffer.size());
        // std::memcpy(ce1, buffer.data(), buffer.size());
        // std::memcpy(ce2, buffer.data(), buffer.size());
        // std::memcpy(ce3, buffer.data(), buffer.size());
        //
        // pager.flush_all();
    }

    int x = 1;

    EXPECT_EQ(x, 1);
}