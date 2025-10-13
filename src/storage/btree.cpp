#include "ds.h"

////////////////////////////////////////////////////////////////

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

inline uint32_t* internal_node_num_keys(char* node) {
    return reinterpret_cast<uint32_t *>(node + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

inline uint32_t* internal_node_right_child(char* node) {
    return reinterpret_cast<uint32_t *>(node + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

inline uint32_t* internal_node_cell(char* node, uint32_t cell_num) {
    return reinterpret_cast<uint32_t *>(node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
}

inline uint32_t* internal_node_child(char* node, uint32_t child_num) {
    uint32_t num_keys = *internal_node_num_keys(node);
    if (child_num > num_keys) {
        std :: cerr << "trying to access a child past num_keys\n";
        exit(EXIT_FAILURE);
    } else if (child_num == num_keys) {
        return internal_node_right_child(node);
    } else {
        return internal_node_cell(node, child_num);
    }
}

inline uint32_t* internal_node_key(char* node, uint32_t key_num) {
    return reinterpret_cast<uint32_t *>(reinterpret_cast<char *> (internal_node_cell(node, key_num)) + INTERNAL_NODE_CHILD_SIZE);
}

inline uint32_t get_node_max_key(char* node) {
    switch (get_node_type(node)) {
        case NodeType::INTERNAL:
            return *internal_node_key(node, *internal_node_num_keys(node) - 1);
        case NodeType::LEAF:
            return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
        default:
            std :: cout << "WRONG MAX KEY\n";
            return 0;
    }
}

inline bool is_node_root(char* node) {
    uint8_t value = *reinterpret_cast<uint8_t *>((node + IS_ROOT_OFFSET));
    return static_cast<bool>(value);
}

inline void set_node_root(char* node, bool is_root) {
    uint8_t value = is_root;
    *(reinterpret_cast<uint8_t *>(node + IS_ROOT_OFFSET)) = value;
}

inline void initialize_internal_node(char* node) {
    set_node_type(node, NodeType::INTERNAL);
    set_node_root(node, false);
    *internal_node_num_keys(node) = 0;
}

inline void initialize_leaf_node(char* node) {
    set_node_type(node, NodeType::LEAF);
    set_node_root(node, false);
    *leaf_node_num_cells(node) = 0;
}

/////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////

void Table::create_new_root(uint32_t right_child_page_num) {
    CacheEntry* CEntryRoot = pager.get_page(root_page_num);
    CacheEntry* CEntryRightChild = pager.get_page(root_page_num);
    uint32_t left_child_page_num = pager.get_unused_page();
    CacheEntry* CEntryLeftChild = pager.get_page(root_page_num);

    char *root = CEntryRoot->data.data();
    char *left_child = CEntryLeftChild->data.data();
    char *right_child = CEntryRightChild->data.data();

    memcpy(left_child, root, PAGE_SIZE);
    set_node_root(left_child, false);

    initialize_internal_node(root);
    set_node_root(root, true);
    *internal_node_num_keys(root) = 1;
    *internal_node_child(root, 0) = left_child_page_num;
    uint32_t left_child_max_key = get_node_max_key(left_child);
    *internal_node_key(root, 0) = left_child_max_key;
    *internal_node_right_child(root) = right_child_page_num;
}

void Table::leaf_node_split_and_insert(Cursor &cursor, uint32_t key, Row *row) {
    CacheEntry* CEntry = pager.get_page(cursor.page_num);
    char *old_data = CEntry->data.data();

    uint32_t new_page_num = pager.get_unused_page();
    CacheEntry* New_CEntry = pager.get_page(cursor.page_num);
    char *new_data = New_CEntry->data.data();

    initialize_leaf_node(new_data);

    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
        char* destination_node;

        if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
            destination_node = new_data;
        } else {
            destination_node = old_data;
        }

        uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        char* destination = leaf_node_cell(destination_node, index_within_node);

        if (i == cursor.cell_num) {
            std :: array<char, ROW_SIZE> buffer = row->serialize();
            memcpy(destination, buffer.data(), ROW_SIZE);
            *leaf_node_key(destination_node, index_within_node) = key;
        } else if (i > cursor.cell_num) {
            memcpy(destination, leaf_node_cell(old_data, i - 1), LEAF_NODE_CELL_SIZE);
        } else {
            memcpy(destination, leaf_node_cell(new_data, i), LEAF_NODE_CELL_SIZE);
        }
    }

    *(leaf_node_num_cells(old_data)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_data)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (is_node_root(old_data)) {
        return create_new_root(new_page_num);
    } else {
        std :: cout << "implement my friend\n";
    }
}

void Table::leaf_node_insert(Cursor &cursor, uint32_t key, Row *row) {
    CacheEntry* CEntry = pager.get_page(cursor.page_num);
    // CEntry->dirty = true;
    char *data = CEntry->data.data();

    const uint32_t num_cells = *leaf_node_num_cells(data);

    if (num_cells >= LEAF_NODE_MAX_CELLS) {
        leaf_node_split_and_insert(cursor, key, row);
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

////////////////////////////////////////////////////////////////

void indent(uint32_t level) {
    for (uint32_t i = 0; i < level; i++) {
        printf("  ");
    }
}

void Table::print_tree(uint32_t page_num, uint32_t indentation_level) {
    CacheEntry* CEntry = pager.get_page(root_page_num);
    char *data = CEntry->data.data();
    uint32_t num_keys, child;

    switch (get_node_type(data)) {
        case (NodeType::LEAF):
            num_keys = *leaf_node_num_cells(data);
            indent(indentation_level);

            std :: cout << "- leaf (size " << num_keys << ")\n";
            for (uint32_t i = 0; i < num_keys; i++) {
                indent(indentation_level + 1);
                std :: cout << "- " << *leaf_node_key(data, i) << "\n";
            }
            break;
        case (NodeType::INTERNAL):
            num_keys = *internal_node_num_keys(data);
            indent(indentation_level);

            std :: cout << "- internal (size " << num_keys << ")\n";
            if (num_keys > 0) {
                for (uint32_t i = 0; i < num_keys; i++) {
                    child = *internal_node_child(data, i);
                    print_tree(child, indentation_level + 1);

                    indent(indentation_level + 1);
                    std :: cout << "- key " << *leaf_node_key(data, i) << "\n";
                }
                child = *internal_node_right_child(data);
                print_tree(child, indentation_level + 1);
            }
            break;
    }
}