#include "ds.h"

uint32_t* leaf_node_num_cells(char* node) {
    return reinterpret_cast<uint32_t *>(node + LEAF_NODE_NUM_CELLS_OFFSET);
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

void initialize_leaf_node(char* node) { *leaf_node_num_cells(node) = 0; }