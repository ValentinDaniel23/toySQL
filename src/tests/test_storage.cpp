#include <gtest/gtest.h>
#include "../OS/specificOS.h"
#include "../storage/ds.h"

TEST(StorageTest, CreateTestDirectory) {
    bool ret = true;

    std :: string path = ROOT;
    path = "testBtree";

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