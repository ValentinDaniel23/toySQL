#include <iostream>
#include "ds.h"

Pager::Pager(const char *filename, size_t cache_size) : cache(cache_size) {
    os :: createFile(filename);
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    file.seekg(0, std::ios::end);
    if (static_cast<uint32_t>(file.tellg()) == 0) {
        std::vector<char> bigBuffer(FILE_PAGES_NUM * PAGE_SIZE, 0);
        file.seekp(0, std::ios::beg);
        file.write(bigBuffer.data(), bigBuffer.size());
    }
}

CacheEntry* Pager::get_page(uint32_t page_num) {
    if (page_num >= FILE_PAGES_NUM)
        throw std::out_of_range("Page number exceeding");

    CacheEntry *CEntry = cache.get_page(page_num);

    if (CEntry != nullptr) {
        return CEntry;
    }

    CEntry = new CacheEntry;
    CEntry->page_num = page_num;

    try {
        file.seekg(page_num * PAGE_SIZE, std::ios::beg);
        if (!file) {
            throw std::ios_base::failure("Failed to seek to page " + std::to_string(page_num));
        }

        file.read(CEntry->data.data(), PAGE_SIZE);
        CEntry->dirty = true;    // to be removed in the future
    } catch (const std::exception& e) {
        delete CEntry;
        throw;
    }

    CacheEntry *ret = cache.add_page(CEntry);
    if (ret != nullptr) {
        file.seekp(ret->page_num * PAGE_SIZE, std::ios::beg);
        file.write(ret->data.data(), PAGE_SIZE);
        delete ret;
    }

    return CEntry;
}

void Pager::flush_all() {
    for (CacheEntry *CEntry : cache.get_cacheList()) {
        if (CEntry->dirty) {
            file.seekp(CEntry->page_num * PAGE_SIZE, std::ios::beg);
            file.write(CEntry->data.data(), PAGE_SIZE);

            CEntry->dirty = false;
        }
    }
}

// Return the found page from cache, else null
CacheEntry* PageCache::get_page(uint32_t page_num) {
    auto obj = page_map.find(page_num);

    if (obj != page_map.end()) {
        lru_list.remove(obj->second);
        lru_list.push_front(obj->second);
        return obj->second;
    }

    return nullptr;;
}

// If the list is already full, first evict the cache once. Always add the new page entry.
// The returned page should be flushed and deleted.
CacheEntry* PageCache::add_page(CacheEntry *page) {
    CacheEntry *CEntry = nullptr;

    if (page_map.size() >= max_pages) {
        CEntry = evict();
    }

    page_map[page->page_num] = page;
    lru_list.push_front(page);

    return CEntry;
}

// If no page left, return null, else remove the oldest page and return it.
// The returned page should be flushed and deleted.
CacheEntry* PageCache::evict() {
    if (lru_list.empty())
        return nullptr;
    CacheEntry* lru = lru_list.back();
    page_map.erase(lru->page_num);
    lru_list.pop_back();
    return lru;
}