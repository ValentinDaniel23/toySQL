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

inline NodeType get_node_type(char* node) {
    uint8_t value = *reinterpret_cast<uint8_t *>(node + NODE_TYPE_OFFSET);
    return static_cast<NodeType>(value);
}

inline void set_node_type(char* node, NodeType type) {
    *reinterpret_cast<uint8_t *>(node + NODE_TYPE_OFFSET) = static_cast<uint8_t>(type);
}

void init_leaf_node(char* node) {
    set_node_type(node, NodeType::LEAF);
    *leaf_node_num_cells(node) = 0;
}

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

Cursor Table::leaf_node_find(uint32_t page_num, uint32_t key) {
    CacheEntry* CEntry = pager.get_page(root_page_num);
    char *data = CEntry->data.data();

    uint32_t num_cells = *leaf_node_num_cells(data);
    Cursor cursor = make_cursor(page_num);

    if (num_cells == 0) {
        cursor.cell_num = 0;
        return cursor;
    }

    uint32_t min_index = 0;
    uint32_t max_index = num_cells - 1;
    uint32_t ans = num_cells;

    while (min_index <= max_index) {
        uint32_t index = (min_index + max_index) / 2;
        uint32_t key_at_index = *leaf_node_key(data, index);

        if (key == key_at_index) {
            cursor.cell_num = index;
            return cursor;
        }

        if (key < key_at_index) {
            if (index == 0) {
                ans = index;
                break;
            }
            max_index = index - 1;
            ans = index;
        } else {
            min_index = index + 1;
        }
    }

    cursor.cell_num = ans;
    return cursor;
}

Cursor Table::table_find(uint32_t key) {
    CacheEntry* CEntry = pager.get_page(root_page_num);

    if (get_node_type(CEntry->data.data()) == NodeType::LEAF) {
        return leaf_node_find(root_page_num, key);
    } else {
        std :: cout << "to implement searching/n";
    }

    // change here
    return make_cursor();
}

void Table::initialize_leaf_node(char *data) {
    init_leaf_node(data);
}
