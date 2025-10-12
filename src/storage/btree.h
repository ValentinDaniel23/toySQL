#ifndef TOYSQL_BTREE_H
#define TOYSQL_BTREE_H

constexpr uint32_t FILE_PAGES_NUM = 128;
constexpr uint32_t PAGE_SIZE = 4096;

struct Row {
    uint32_t id{};
    uint8_t text_size{};
    std::array<char, 255> text{};

    static constexpr std::size_t ROW_SIZE = sizeof(id) + sizeof(text_size) + sizeof(text);   // alignment

    [[nodiscard]] std::array<char, ROW_SIZE> serialize() const;
    static Row deserialize(const std::array<char, ROW_SIZE>& buffer);
};

#define COLUMN_STRING 255

constexpr size_t ID_OFFSET       = offsetof(Row, id);
constexpr size_t TEXT_SIZE_OFFSET = offsetof(Row, text_size);
constexpr size_t TEXT_OFFSET = offsetof(Row, text);
constexpr size_t ROW_SIZE = Row::ROW_SIZE;

enum class NodeType : uint8_t {
    INTERNAL,
    LEAF
};

/*
PAGE LAYOUT SCHEMA (for B+Tree nodes)

Each page = PAGE_SIZE bytes (e.g., 4096)

-------------------------------------------------------
| COMMON HEADER (6 bytes)                             |
|-----------------------------------------------------|
| node_type   : uint8_t   (0=internal, 1=leaf)       | 0
| is_root     : uint8_t   (1=root, 0=not root)       | 1
| parent_page : uint32_t  (page number of parent)    | 2
-------------------------------------------------------
| LEAF NODE HEADER (8 bytes)                         |
| (only if leaf)                                     |
|-----------------------------------------------------|
| num_cells  : uint32_t   (number of key/value pairs)| 6
-------------------------------------------------------
| INTERNAL NODE HEADER (8 bytes)                     |
| (only if internal)                                 |
|-----------------------------------------------------|
| num_keys    : uint32_t   (number of keys)          | 6
| right_child : uint32_t   (page number of rightmost child)| 10
*/

//  COMMON HEADER  //
constexpr size_t NODE_TYPE_SIZE = sizeof(NodeType);
constexpr size_t IS_ROOT_SIZE = sizeof(uint8_t);
constexpr size_t PARENT_POINTER_SIZE = sizeof(uint32_t);

constexpr size_t NODE_TYPE_OFFSET      = 0;
constexpr size_t IS_ROOT_OFFSET        = NODE_TYPE_SIZE;
constexpr size_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;

constexpr size_t COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

//  LEAF HEADER  //
// const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
constexpr size_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
constexpr size_t LEAF_NODE_HEADER_SIZE = LEAF_NODE_NUM_CELLS_OFFSET + sizeof(uint32_t);

//  INTERNAL HEADER  //
constexpr size_t INTERNAL_RIGHT_CHILD_OFFSET = COMMON_NODE_HEADER_SIZE + sizeof(uint32_t);
// constexpr size_t INTERNAL_NODE_HEADER_SIZE = INTERNAL_RIGHT_CHILD_OFFSET + sizeof(uint32_t);

//  LEAF NODE BODY LAYOUT  //
constexpr uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
constexpr uint32_t LEAF_NODE_KEY_OFFSET = 0;
constexpr uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
constexpr uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
constexpr uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
constexpr uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
constexpr uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

#endif //TOYSQL_BTREE_H