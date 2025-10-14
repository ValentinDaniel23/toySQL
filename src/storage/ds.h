#ifndef TOYSQL_DS_H
#define TOYSQL_DS_H

#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <list>
#include <iostream>
#include "../OS/specificOS.h"
#include "btree.h"

void initialize_leaf_node(char* node);
void set_node_root(char* node, bool is_root);

struct CacheEntry {
    uint32_t page_num;
    std::array<char, PAGE_SIZE> data;
    bool dirty = false;
};

class PageCache {
    friend class Pager;

    PageCache(size_t max) : max_pages(max) {}

    CacheEntry* get_page(uint32_t page_num);
    CacheEntry* add_page(CacheEntry* page);
    CacheEntry* evict();

    const std::list<CacheEntry*>& get_cacheList() {
        return lru_list;
    }

    void mark_dirty(CacheEntry* entry) {
        entry->dirty = true;
    }

    ~PageCache() {
        for (auto& [_, page] : page_map)
            delete page;
    }

    size_t max_pages;
    std::unordered_map<uint32_t, CacheEntry*> page_map;
    std::list<CacheEntry*> lru_list;
};

class Pager {
public:
    friend class Table;

    CacheEntry* get_page(uint32_t page_num);
    void flush_all();

    // Eventually after we implement deletion, some pages may become empty and their page numbers unused.
    // To be more efficient, we could re-allocate those free pages.
    uint32_t get_unused_page() { return num_pages; }

private:
    Pager(const char* filename, size_t cache_size);

    std::fstream file;
    PageCache cache;
    uint32_t num_pages = 0;
};

class Cursor {
public:
    friend class Table;

    Pager& pager;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table;

    [[nodiscard]] Row* value() const;
    void advance();

private:
    [[nodiscard]] CacheEntry* current_page() const;

    Cursor(Pager &pager) : pager(pager), page_num(0), cell_num(0), end_of_table(false) {}
    Cursor(Pager &pager, uint32_t page_num, uint32_t cell_num, bool end = false)
        : pager(pager), page_num(page_num), cell_num(cell_num), end_of_table(end) {}
};

class Table {
public:
    Pager pager;
    const char *filename;
    uint32_t root_page_num = 0;

    Table(const char* filename, size_t cache_size) : pager(filename, cache_size), filename{filename} {}

    void create_new_root(uint32_t right_child_page_num);
    void leaf_node_split_and_insert(Cursor& cursor, uint32_t key, Row *row);
    void leaf_node_insert(Cursor& cursor, uint32_t key, Row *row);
    Cursor table_find(uint32_t key);
    Cursor leaf_node_find(uint32_t page_num, uint32_t key);
    Cursor internal_node_find(uint32_t page_num, uint32_t key);

    void print_tree(uint32_t page_num, uint32_t indentation_level);

    Cursor make_cursor(uint32_t page_num = 0, uint32_t cell_num = 0) {
        return Cursor{pager, page_num, cell_num};
    }
};

#endif //TOYSQL_DS_H