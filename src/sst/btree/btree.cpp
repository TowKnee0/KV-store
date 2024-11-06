#include "btree.h"
#include <cmath>

#include <iostream>

#include "../../io/io_handler/io_handler.h"

double log(double x, double b){
    return log(x) / log(b);
}

BTree::BTree(string filename, string dbname, shared_ptr<IO> handler) {
  this->filename = filename;
  this->io_handler = handler;
  this->B = io_handler->getPageSize();
}

int BTree::write(const vector<pair<int, int>> &data) {
  if (io_handler->writeToFile(filename + this->LEAF_NODE_SUFFIX, data) == -1) {
    return EXIT_FAILURE;
  }
  this->num_data_pages = io_handler->numPagesInFile(filename);
  this->num_data = data.size();
  return EXIT_SUCCESS;
}

void BTree::contructInternalNodes() {
    // Determine how many levels we need to construct
    int num_levels = ceil(log(this->num_data_pages, this->B));

    // Vector which stores an array from level -> pair<value, page>
    // The vector does not separate into different nodes yet, it just has all values.
    // Afterwards we will need to split into nodes when writing to disk.
    vector<vector<pair<int, int>>> levels = vector<vector<pair<int, int>>>(num_levels - 1); // -1 dont need leafs
    for (int i = 1; i < this->num_data_pages + 1; i++) {
        // Construct the internal nodes
        vector<pair<int, int>> page = io_handler->readPageFromFile(filename + this->LEAF_NODE_SUFFIX, i).value();
        int last_value = page[page.size() - 1].first;
        levels[0].push_back(make_pair(last_value, i));
    }

    int level = 1;
    while (level < num_levels - 1) {
        int prev_level_size = levels[level - 1].size();
        // Every B values in the previous level, we need add a new value to the current level.
        // Pages are 1 indexed to keep it consistent with the io handler.
        for (int i = B; i < prev_level_size; i += this->B) {
            levels[level].push_back(make_pair(levels[level - 1][i].first, i / B));
        }
    }

}