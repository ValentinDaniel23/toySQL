#ifndef TOYSQL_DS_H
#define TOYSQL_DS_H

#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <list>
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

constexpr uint32_t FILE_PAGES_NUM = 128;
constexpr uint32_t PAGE_SIZE = 4096;

struct CacheEntry {
    uint32_t page_num;
    std::array<char, PAGE_SIZE> data;
    bool dirty = false;
};

class PageCache {
    friend class Pager;

    PageCache(size_t max) : max_pages(max) {}

    // read tuple, index
    // write tuple, index

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
    Pager(const char* filename, size_t cache_size);
    CacheEntry* get_page(uint32_t page_num);
    void flush_all();

private:
    std::fstream file;
    PageCache cache;
};

class Table {
    Pager pager;

    Table(const char* filename, size_t cache_size) : pager(filename, cache_size) {}
};

#endif //TOYSQL_DS_H