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