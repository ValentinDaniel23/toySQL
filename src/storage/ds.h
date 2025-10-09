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
    // Pager(const char* filename, size_t cache_size);
    // CacheEntry* get_page(uint32_t page_num);
    // void flush_all();

    void flush_all();

private:
    Pager(const char* filename, size_t cache_size);
    CacheEntry* get_page(uint32_t page_num);

    std::fstream file;
    PageCache cache;
};

class Table {
public:

    Pager pager;
    const char *filename;

    Table(const char* filename, size_t cache_size) : filename{filename}, pager(filename, cache_size) {}
};

#endif //TOYSQL_DS_H