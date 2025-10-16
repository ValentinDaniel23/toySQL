#include "ds.h"

////////////////////////////////////////////////////////////////

uint32_t* leaf_node_num_cells(char* node) {
    return reinterpret_cast<uint32_t *>(node + LEAF_NODE_NUM_CELLS_OFFSET);
}

uint32_t* leaf_node_next_leaf(char* node) {
    return reinterpret_cast<uint32_t *>(node + LEAF_NODE_NEXT_LEAF_OFFSET);
}

char* leaf_node_cell(char* node, uint32_t cell_num) {
    return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t* leaf_node_key(char* node, uint32_t cell_num) {
    return reinterpret_cast<uint32_t *>(leaf_node_cell(node, cell_num));
}

char* leaf_node_value(char* node, uint32_t cell_num) {
    return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

NodeType get_node_type(char* node) {
    uint8_t value = *reinterpret_cast<uint8_t *>(node + NODE_TYPE_OFFSET);
    return static_cast<NodeType>(value);
}

void set_node_type(char* node, NodeType type) {
    *reinterpret_cast<uint8_t *>(node + NODE_TYPE_OFFSET) = static_cast<uint8_t>(type);
}

uint32_t* internal_node_num_keys(char* node) {
    return reinterpret_cast<uint32_t *>(node + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

uint32_t* internal_node_right_child(char* node) {
    return reinterpret_cast<uint32_t *>(node + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t* internal_node_cell(char* node, uint32_t cell_num) {
    return reinterpret_cast<uint32_t *>(node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
}

uint32_t* internal_node_child(char* node, uint32_t child_num) {
    uint32_t num_keys = *internal_node_num_keys(node);
    if (child_num > num_keys) {
        std :: cerr << "trying to access a child past num_keys in internal_node_child\n";
        exit(EXIT_FAILURE);
    } else if (child_num == num_keys) {
        uint32_t* right_child = internal_node_right_child(node);
        if (*right_child == INVALID_PAGE_NUM) {
            printf("Tried to access right child of node, but was invalid page\n");
            exit(EXIT_FAILURE);
        }
        return right_child;
    } else {
        uint32_t* child = internal_node_cell(node, child_num);
        if (*child == INVALID_PAGE_NUM) {
            printf("Tried to access child %d of node, but was invalid page\n", child_num);
            exit(EXIT_FAILURE);
        }
        return child;
    }
}

uint32_t* internal_node_key(char* node, uint32_t key_num) {
    return reinterpret_cast<uint32_t *>(reinterpret_cast<char *> (internal_node_cell(node, key_num)) + INTERNAL_NODE_CHILD_SIZE);
}

void update_internal_node_key(char* node, uint32_t old_child_index, uint32_t new_key) {
    *internal_node_key(node, old_child_index) = new_key;
}

uint32_t get_node_max_key(char* node) {
    switch (get_node_type(node)) {
        case NodeType::INTERNAL:
            return *internal_node_key(node, *internal_node_num_keys(node) - 1);
        case NodeType::LEAF:
            return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
        default:
            std :: cerr << "wrong max key in get_node_max_key";
            exit(EXIT_FAILURE);
    }
}

uint32_t* node_parent(char* node) {
    return reinterpret_cast<uint32_t *>(node + PARENT_POINTER_OFFSET);
}

bool is_node_root(char* node) {
    uint8_t value = *reinterpret_cast<uint8_t *>((node + IS_ROOT_OFFSET));
    return static_cast<bool>(value);
}

void set_node_root(char* node, bool is_root) {
    uint8_t value = is_root;
    *(reinterpret_cast<uint8_t *>(node + IS_ROOT_OFFSET)) = value;
}

void initialize_internal_node(char* node) {
    set_node_type(node, NodeType::INTERNAL);
    set_node_root(node, false);
    *internal_node_num_keys(node) = 0;
    *internal_node_right_child(node) = INVALID_PAGE_NUM;
}

void initialize_leaf_node(char* node) {
    set_node_type(node, NodeType::LEAF);
    set_node_root(node, false);
    *leaf_node_num_cells(node) = 0;
    *leaf_node_next_leaf(node) = 0;
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
    char *data = CEntry->data.data();

    cell_num += 1;
    if (cell_num >= (*leaf_node_num_cells(CEntry->data.data()))) {
        uint32_t next_page_num = *leaf_node_next_leaf(data);

        if (next_page_num == 0) {
            end_of_table = true;
        } else {
            page_num = next_page_num;
            cell_num = 0;
        }
    }
}

/////////////////////////////////////////////////////////////

uint32_t Table::get_node_max_key(char* node) {
    if (get_node_type(node) == NodeType::LEAF) {
        uint32_t num_cells = *leaf_node_num_cells(node);
        if (num_cells == 0) return 0;
        return *leaf_node_key(node, num_cells - 1);
    }

    uint32_t num_keys = *internal_node_num_keys(node);
    if (num_keys == 0) {
        uint32_t page = *internal_node_right_child(node);
        CacheEntry *CRight_Child = pager.get_page(page);
        char* right_child = CRight_Child->data.data();
        return get_node_max_key(right_child);
    }
    return *internal_node_key(node, num_keys - 1);
}

void Table::internal_node_split_and_insert(uint32_t parent_page_num, uint32_t child_page_num) {
    uint32_t old_page_num = parent_page_num;

    CacheEntry *COld = pager.get_page(parent_page_num);
    char *old_node = COld->data.data();
    uint32_t old_max = get_node_max_key(old_node);

    CacheEntry *CChild = pager.get_page(child_page_num);
    char* child = CChild->data.data();
    uint32_t child_max = get_node_max_key(child);

    uint32_t new_page_num = pager.get_unused_page();

    uint32_t splitting_root = is_node_root(old_node);

    char* parent;
    char* new_node;

    if (splitting_root) {
        create_new_root(new_page_num);

        CacheEntry *CParent = pager.get_page(root_page_num);
        parent = CParent->data.data();

        old_page_num = *internal_node_child(parent,0);

        CacheEntry *CNOld = pager.get_page(old_page_num);
        old_node = CNOld -> data.data();
    } else {
        CacheEntry *CParent = pager.get_page(*node_parent(old_node));
        CacheEntry *CN = pager.get_page(new_page_num);

        parent = CParent->data.data();
        new_node = CN->data.data();
        initialize_internal_node(new_node);
    }

    uint32_t* old_num_keys = internal_node_num_keys(old_node);

    uint32_t cur_page_num = *internal_node_right_child(old_node);
    CacheEntry *CCur = pager.get_page(cur_page_num);
    char* cur = CCur->data.data();

    internal_node_insert(new_page_num, cur_page_num);
    *node_parent(cur) = new_page_num;
    *internal_node_right_child(old_node) = INVALID_PAGE_NUM;

    for (int i = INTERNAL_NODE_MAX_CELLS - 1; i > INTERNAL_NODE_MAX_CELLS / 2; i--) {
        cur_page_num = *internal_node_child(old_node, i);

        CCur = pager.get_page(cur_page_num);
        cur = CCur->data.data();

        internal_node_insert(new_page_num, cur_page_num);
        *node_parent(cur) = new_page_num;

        (*old_num_keys)--;
    }

    *internal_node_right_child(old_node) = *internal_node_child(old_node,*old_num_keys - 1);
    (*old_num_keys)--;

    uint32_t max_after_split = get_node_max_key(old_node);

    uint32_t destination_page_num = child_max < max_after_split ? old_page_num : new_page_num;

    internal_node_insert(destination_page_num, child_page_num);
    *node_parent(child) = destination_page_num;

    update_internal_node_key(parent, old_max, get_node_max_key(old_node));

    if (!splitting_root) {
        internal_node_insert(*node_parent(old_node),new_page_num);
        *node_parent(new_node) = *node_parent(old_node);
    }
}

void Table::internal_node_insert(uint32_t parent_page_num, uint32_t child_page_num) {
    CacheEntry* CParent = pager.get_page(parent_page_num);
    char* parent = CParent->data.data();

    CacheEntry* CChild = pager.get_page(child_page_num);
    char* child = CChild->data.data();

    uint32_t child_max_key = get_node_max_key(child);
    uint32_t index = internal_node_find_child(parent, child_max_key);

    uint32_t original_num_keys = *internal_node_num_keys(parent);

    if (original_num_keys >= INTERNAL_NODE_MAX_CELLS) {
        internal_node_split_and_insert(parent_page_num, child_page_num);
        return;
    }

    uint32_t right_child_page_num = *internal_node_right_child(parent);

    if (right_child_page_num == INVALID_PAGE_NUM) {
        *internal_node_right_child(parent) = child_page_num;
        return;
    }

    CacheEntry* CRight = pager.get_page(right_child_page_num);
    char* right_child = CRight->data.data();

    *internal_node_num_keys(parent) = original_num_keys + 1;

    if (child_max_key > get_node_max_key(right_child)) {
        *internal_node_child(parent, original_num_keys) = right_child_page_num;
        *internal_node_key(parent, original_num_keys) = get_node_max_key(right_child);
        *internal_node_right_child(parent) = child_page_num;
    } else {
        for (uint32_t i = original_num_keys; i > index; i--) {
            void* destination = internal_node_cell(parent, i);
            void* source = internal_node_cell(parent, i - 1);
            memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
        }
        *internal_node_child(parent, index) = child_page_num;
        *internal_node_key(parent, index) = child_max_key;
    }
}

uint32_t Table::internal_node_find_child(char* node, uint32_t key) {
    uint32_t num_keys = *internal_node_num_keys(node);

    uint32_t ans = num_keys;

    if (num_keys != 0) {
        uint32_t min_index = 0;
        uint32_t max_index = num_keys - 1;

        while (min_index <= max_index) {
            uint32_t index = (min_index + max_index) / 2;
            uint32_t key_to_right = *internal_node_key(node, index);

            if (key <= key_to_right) {
                ans = index;
                if (index == 0)
                    break;
                max_index = index - 1;
            } else {
                min_index = index + 1;
            }
        }
    }

    return ans;
}

Cursor Table::internal_node_find(uint32_t page_num, uint32_t key) {
    CacheEntry* CEntry = pager.get_page(page_num);
    char* data = CEntry->data.data();

    uint32_t child_index = internal_node_find_child(data, key);
    uint32_t child_num = *internal_node_child(data, child_index);
    CacheEntry* CEntryChinld = pager.get_page(child_num);
    char* dataChild = CEntryChinld->data.data();

    switch (get_node_type(dataChild)) {
        case NodeType::LEAF:
            return leaf_node_find(child_num, key);
        case NodeType::INTERNAL:
            return internal_node_find(child_num, key);
        default:
            std :: cerr << "error child type\n";
            exit(EXIT_FAILURE);
    }
}

void Table::create_new_root(uint32_t right_child_page_num) {
    CacheEntry* CEntryRoot = pager.get_page(root_page_num);
    CacheEntry* CEntryRightChild = pager.get_page(right_child_page_num);
    uint32_t left_child_page_num = pager.get_unused_page();
    CacheEntry* CEntryLeftChild = pager.get_page(left_child_page_num);

    char *root = CEntryRoot->data.data();
    char *left_child = CEntryLeftChild->data.data();
    char *right_child = CEntryRightChild->data.data();

    if (get_node_type(root) == NodeType::INTERNAL) {
        initialize_internal_node(right_child);
        initialize_internal_node(left_child);
    }

    memcpy(left_child, root, PAGE_SIZE);
    set_node_root(left_child, false);

    if (get_node_type(left_child) == NodeType::INTERNAL) {
        CacheEntry *CChild;
        char* child;

        uint32_t internal_right_child = *internal_node_right_child(left_child);
        uint32_t childs = *internal_node_num_keys(left_child);

        for (int i = 0; i < childs; i++) {
            CChild = pager.get_page(*internal_node_child(left_child, i));
            child = CChild->data.data();
            *node_parent(child) = left_child_page_num;
        }

        CChild = pager.get_page(internal_right_child);
        child = CChild->data.data();
        *node_parent(child) = left_child_page_num;
    }

    initialize_internal_node(root);
    set_node_root(root, true);
    *internal_node_num_keys(root) = 1;
    *internal_node_child(root, 0) = left_child_page_num;
    uint32_t left_child_max_key = get_node_max_key(left_child);
    *internal_node_key(root, 0) = left_child_max_key;
    *internal_node_right_child(root) = right_child_page_num;
    *node_parent(left_child) = root_page_num;
    *node_parent(right_child) = root_page_num;
}

void Table::leaf_node_split_and_insert(Cursor &cursor, uint32_t key, Row *row) {
    CacheEntry* CEntry = pager.get_page(cursor.page_num);
    char *old_data = CEntry->data.data();
    uint32_t old_max = get_node_max_key(old_data);

    uint32_t new_page_num = pager.get_unused_page();
    CacheEntry* New_CEntry = pager.get_page(new_page_num);
    char *new_data = New_CEntry->data.data();

    initialize_leaf_node(new_data);

    *node_parent(new_data) = *node_parent(old_data);
    *leaf_node_next_leaf(new_data) = *leaf_node_next_leaf(old_data);
    *leaf_node_next_leaf(old_data) = new_page_num;

    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
        char* destination_node;

        if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
            destination_node = new_data;
        } else {
            destination_node = old_data;
        }

        uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        char* destination = leaf_node_cell(destination_node, index_within_node);
        char* destinationValue = leaf_node_value(destination_node, index_within_node);

        if (i == cursor.cell_num) {
            std :: array<char, ROW_SIZE> buffer = row->serialize();
            memcpy(destinationValue, buffer.data(), ROW_SIZE);
            *reinterpret_cast<uint32_t*>(destination) = key;
        } else if (i > cursor.cell_num) {
            memcpy(destination, leaf_node_cell(old_data, i - 1), LEAF_NODE_CELL_SIZE);
        } else {
            memcpy(destination, leaf_node_cell(old_data, i), LEAF_NODE_CELL_SIZE);
        }
    }

    *(leaf_node_num_cells(old_data)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_data)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (is_node_root(old_data)) {
        return create_new_root(new_page_num);
    } else {
        uint32_t parent_page_num = *node_parent(old_data);
        uint32_t new_max = get_node_max_key(old_data);

        CacheEntry* CParent = pager.get_page(cursor.page_num);
        char* parent = CParent->data.data();
        update_internal_node_key(parent, internal_node_find_child(parent, old_max), new_max);
        internal_node_insert(parent_page_num, new_page_num);
        return;
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
    CacheEntry* CEntry = pager.get_page(page_num);
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

    switch (get_node_type(CEntry->data.data())) {
        case (NodeType::LEAF):
            return leaf_node_find(root_page_num, key);
        case (NodeType::INTERNAL):
            return internal_node_find(root_page_num, key);
        default:
            std :: cerr << "no valid node type in table_find\n";
            exit(EXIT_FAILURE);
    }
}

Cursor Table::table_start() {
    Cursor cursor = table_find(0);

    CacheEntry* CEntry = pager.get_page(root_page_num);
    char* data = CEntry->data.data();

    uint32_t num_cells = *leaf_node_num_cells(data);
    cursor.end_of_table = (num_cells == 0);
    return cursor;
}

////////////////////////////////////////////////////////////////

void indent(uint32_t level) {
    for (uint32_t i = 0; i < level; i++) {
        std :: cout << "  ";
    }
}

void Table::print_tree(uint32_t page_num, uint32_t indentation_level) {
    CacheEntry* CEntry = pager.get_page(page_num);
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
                    std :: cout << "- key " << *internal_node_key(data, i) << "\n";
                }
                child = *internal_node_right_child(data);
                print_tree(child, indentation_level + 1);
            }
            break;
        default:
            std :: cerr << "Unknown NodeType in print_tree";
            exit(EXIT_FAILURE);
    }
}