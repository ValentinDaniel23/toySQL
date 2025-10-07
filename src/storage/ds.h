#ifndef TOYSQL_DS_H
#define TOYSQL_DS_H

#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include "../OS/specificOS.h"

#define COLUMN_STRING 255

struct Row {
    uint32_t id{};
    uint8_t text_size{};
    std::array<char, 255> text{};

    static constexpr std::size_t ROW_SIZE = sizeof(id) + sizeof(text_size) + sizeof(text);   // alignment

    [[nodiscard]] std::array<char, ROW_SIZE> serialize() const;
    static Row deserialize(const std::array<char, ROW_SIZE>& buffer);
};

constexpr std::size_t ID_OFFSET       = offsetof(Row, id);
constexpr std::size_t TEXT_SIZE_OFFSET = offsetof(Row, text_size);
constexpr std::size_t TEXT_OFFSET = offsetof(Row, text);
constexpr std::size_t ROW_SIZE = Row::ROW_SIZE;

constexpr uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 400


class Pager {
public:
    std::fstream file;
    uint32_t file_length;
    uint32_t num_pages;
    std::array<void*, TABLE_MAX_PAGES> pages{};

    explicit Pager(const char* filename);

    ~Pager();


    // LEFT HERE
    void* get_page(uint32_t page_num) {
        if (page_num >= TABLE_MAX_PAGES)
            throw std::out_of_range("Page number out of bounds");

        if (pages[page_num] == nullptr) {
            // Allocate memory
            pages[page_num] = new char[PAGE_SIZE];

            // If page exists in file, read it
            if (page_num < num_pages) {
                lseek(file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
                ssize_t bytes_read = read(file_descriptor, pages[page_num], PAGE_SIZE);
                if (bytes_read == -1)
                    throw std::runtime_error("Error reading file");
            } else {
                // New blank page
                std::memset(pages[page_num], 0, PAGE_SIZE);
            }

            if (page_num >= num_pages)
                num_pages = page_num + 1;
        }

        return pages[page_num];
    }

    // void flush_page(uint32_t page_num) {
    //     if (pages[page_num] == nullptr)
    //         throw std::runtime_error("Tried to flush null page");
    //
    //     lseek(file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
    //     ssize_t bytes_written = write(file_descriptor, pages[page_num], PAGE_SIZE);
    //     if (bytes_written == -1)
    //         throw std::runtime_error("Error writing page to file");
    // }
};


// class Table {
//     typedef struct {
//         int file_descriptor;
//         uint32_t file_length;
//         uint32_t num_pages;
//         void* pages[TABLE_MAX_PAGES];
//     } Pager;
//
//     Pager pager;
//     uint32_t root_page_num;
//
//     explicit Table(const char* path) {
//
//     }
// };
//
//
// Pager* pager_open(const char* filename) {
//     int fd = open(filename,
//                   O_RDWR |      // Read/Write mode
//                       O_CREAT,  // Create file if it does not exist
//                   S_IWUSR |     // User write permission
//                       S_IRUSR   // User read permission
//                   );
//
//     if (fd == -1) {
//         printf("Unable to open file\n");
//         exit(EXIT_FAILURE);
//     }
//
//     off_t file_length = lseek(fd, 0, SEEK_END);
//
//     Pager* pager = malloc(sizeof(Pager));
//     pager->file_descriptor = fd;
//     pager->file_length = file_length;
//     pager->num_pages = (file_length / PAGE_SIZE);
//
//     if (file_length % PAGE_SIZE != 0) {
//         printf("Db file is not a whole number of pages. Corrupt file.\n");
//         exit(EXIT_FAILURE);
//     }
//
//     for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
//         pager->pages[i] = NULL;
//     }
//
//     return pager;
// }
//
// Table* db_open(const char* filename) {
//     Pager* pager = pager_open(filename);
//
//     Table* table = malloc(sizeof(Table));
//     table->pager = pager;
//     table->root_page_num = 0;
//
//     if (pager->num_pages == 0) {
//         // New database file. Initialize page 0 as leaf node.
//         void* root_node = get_page(pager, 0);
//         initialize_leaf_node(root_node);
//         set_node_root(root_node, true);
//     }
//
//     return table;
// }


#endif //TOYSQL_DS_H