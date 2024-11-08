#include "btree.h"
#include <cmath>

#include <iostream>
#include "../../io/io_handler/io_handler.h"

using namespace std;

double log(double x, double b){
    return log(x) / log(b);
}

/**
 * @brief Computes the height of a B-tree.
 * Log_(B+1)(N) -> Branching factor is 1 more than the number of keys in a node.
 *
 * @param num_keys The number of keys in the B-tree.
 * @param B Number of keys per node
 * @return The height of the B-tree.
 */
int computeHeight(int num_keys, int B){
  return ceil(log(num_keys, B + 1));
}

/**
 * @brief Calculates the number of nodes required for a B-tree with num_keys.
 *
 * @param num_keys The total number of keys to be stored in the B-tree.
 * @param B Number of keys per node
 * @return The number of nodes required to store the given number of keys.
 */
int numNodes(int num_keys, int B){
  return ceil(num_keys / (B + 1));
}

/**
 * @brief Calculates the number of keys on the previous layer of a B-tree.
 * 
 * @param current_keys The number of keys on the current layer.
 * @param B Number of keys per node
 * @return The number of keys on the previous layer.
 */
int numKeysOnPrevLayer(int current_keys, int B){
  return ceil(current_keys / (B + 1));
}

/**
 * @brief Computes the starting index of a given layer in a B-tree.
 *
 * @param layer The layer for which the starting index is to be calculated.
 * @param num_keys The total number of keys in the B-tree.
 * @param B Number of keys per node
 * @return The starting index of the specified layer.
 */
int layerStartingIndex(int layer, int num_keys, int B){
  int block_index = 0;
  while (layer > 0){
    block_index += numNodes(num_keys, B);
    num_keys = numKeysOnPrevLayer(num_keys, B);
    layer--;
  }
  return block_index;
}

BTree::BTree(string filename, string dbname, shared_ptr<IO> handler) {
  this->filename = filename;
  this->io_handler = handler;
  this->B = io_handler->getPageSize() / io_handler->getPairSize();
}

int BTree::write(const vector<pair<int, int>> &data) {
  if (io_handler->writeToFile(filename, data) == -1) {
    return EXIT_FAILURE;
  }
  this->num_leaf_pages = io_handler->numPagesInFile(filename);
  this->num_data = data.size();
  this->height = computeHeight(this->num_data, this->B);
  return EXIT_SUCCESS;
}

void BTree::contructInternalNodes() {
    // Number of elements would be the first element in the next layer.
    int total_btree_size = layerStartingIndex(this->height, this->num_data, this->B);
    int internal_btree_size = total_btree_size - this->num_data;

    vector<pair<int, int>> internal_nodes(internal_btree_size, {this->INF, this->INF});

    for (int height = 1; height < this->height; height++) {
        int next_layer_start = layerStartingIndex(height + 1, this->num_data, this->B);
        int this_layer_start = layerStartingIndex(height, this->num_data, this->B);
        int elements_in_this_layer = next_layer_start - this_layer_start;

        for (int element_index = 0; element_index < elements_in_this_layer; element_index++) {
            int block_index = element_index / (this->B); // 0 Indexed
            int block_element_index = element_index - block_index * this->B; // 0 indexed.
            int right_child_block = block_index * (this->B + 1) + block_element_index + 1;

            // Keep calculating until we reach the leaf node.
            for (int i = 0; i < height - 1; i++) {
                right_child_block = right_child_block * (this->B + 1);
            }


            int child_page_number = right_child_block + 1; // 1 indexed

            // TODO: Get from buffer first.
            optional<vector<pair<int, int>>> leaf_page = this->io_handler->readPageFromFile(this->filename, child_page_number);
            if (!leaf_page.has_value()) {
                // Handle error
            }
            vector<pair<int, int>> leaf_data = leaf_page.value();
            internal_nodes[element_index] = leaf_data[0];
        }
    }
}