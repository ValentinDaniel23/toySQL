#include "ds.h"

inline uint32_t* leaf_node_num_cells(char* node) {
    return reinterpret_cast<uint32_t *>(node + LEAF_NODE_NUM_CELLS_OFFSET);
}

inline char* leaf_node_cell(char* node, uint32_t cell_num) {
    return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

inline uint32_t* leaf_node_key(char* node, uint32_t cell_num) {
    return reinterpret_cast<uint32_t *>(leaf_node_cell(node, cell_num));
}

inline char* leaf_node_value(char* node, uint32_t cell_num) {
    return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

inline void initialize_leaf_node(char* node) { *leaf_node_num_cells(node) = 0; }

CacheEntry* Cursor::current_page() const {
    return pager.get_page(page_num);
}

Row* Cursor::value() const {
    CacheEntry* CEntry = current_page();
    // CEntry->dirty = true;

    return reinterpret_cast<Row*> (leaf_node_value(CEntry->data.data(), cell_num));
}

void Cursor::advance() {
    CacheEntry* CEntry = current_page();

    cell_num += 1;
    if (cell_num >= (*leaf_node_num_cells(CEntry->data.data()))) {
        end_of_table = true;
    }
}

void Table::leaf_node_insert(Cursor &cursor, uint32_t key, Row *row) {
    CacheEntry* CEntry = pager.get_page(cursor.page_num);
    // CEntry->dirty = true;
    char *data = CEntry->data.data();

    const uint32_t num_cells = *leaf_node_num_cells(data);
    if (num_cells >= LEAF_NODE_MAX_CELLS) {
        // Node full
        std :: cout << "to implement\n";
        return;
    }

    if (cursor.cell_num < num_cells) {
        for (uint32_t i = num_cells; i > cursor.cell_num; i--) {
            memcpy(leaf_node_cell(data, i), leaf_node_cell(data, i - 1),
                   LEAF_NODE_CELL_SIZE);
        }
    }

    *(leaf_node_num_cells(data)) += 1;
    *(leaf_node_key(data, cursor.cell_num)) = key;
    std :: array<char, ROW_SIZE> buffer = row->serialize();
    memcpy(leaf_node_value(data, cursor.cell_num), buffer.data(), ROW_SIZE);
}