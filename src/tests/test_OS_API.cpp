#include <gtest/gtest.h>
#include "../OS/specificOS.h"

TEST(OS_APITest, create_and_remove_DIR) {
    bool ret = true;

    std :: string path = ROOT;
    path += "test";

    EXPECT_EQ(os::createDir(path), true);
    EXPECT_EQ(os::removeDir(path), true);
}

TEST(OS_APITest, create_and_remove_FILES) {
    bool ret = true;

    std :: string path = ROOT;
    path += "test1";

    EXPECT_EQ(os::createDir(path), true);

    std :: string file1 = path + OS_SEP + "file.txt";
    std :: string file2 = path + OS_SEP + "passwords.txt";
    std :: string file3 = path + OS_SEP + "logs.txt";

    EXPECT_EQ(os::createFile(file1), true);
    EXPECT_EQ(os::createFile(file2), true);
    EXPECT_EQ(os::createFile(file3), true);

    EXPECT_EQ(os::fileExists(file1), true);
    EXPECT_EQ(os::fileExists(file2), true);
    EXPECT_EQ(os::fileExists(file3), true);
    EXPECT_EQ(os::fileExists(path + OS_SEP + "KING.txt"), false);
}

TEST(OS_APITest, listFiles) {
    bool ret = true;

    std :: string path = ROOT;
    path += "test1";

    EXPECT_EQ(os::createDir(path), true);

    std :: string file1 = path + OS_SEP + "file.txt";
    std :: string file2 = path + OS_SEP + "passwords.txt";
    std :: string file3 = path + OS_SEP + "logs.txt";

    EXPECT_EQ(os::fileExists(file1), true);
    EXPECT_EQ(os::fileExists(file2), true);
    EXPECT_EQ(os::fileExists(file3), true);
    EXPECT_EQ(os::fileExists(path + OS_SEP + "KING.txt"), false);

    std :: vector<std :: string> files = os :: listFiles(path);

    std :: unordered_map<std :: string, int> exist = {
        {"file.txt", 0},
        {"passwords.txt", 0},
        {"logs.txt", 0}
    };

    for (const std :: string& file : files) {
        EXPECT_EQ((exist.find(file) != exist.end()), true);
    }
}