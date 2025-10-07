#include "ds.h"

Pager::Pager(const char* filename) {
    os :: createFile(filename);
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    file.seekg(0, std::ios::end);
    file_length = static_cast<uint32_t>(file.tellg());
    num_pages = file_length / PAGE_SIZE;
    if (file_length % PAGE_SIZE != 0)
        throw std::runtime_error("Corrupt file: not a whole number of pages");

    pages.fill(nullptr);
}

Pager::~Pager() {
    for (auto* page : pages) {
        if (page != nullptr) {
            delete[] static_cast<char*>(page);
        }
    }
}
