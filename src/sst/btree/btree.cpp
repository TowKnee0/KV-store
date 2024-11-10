#include "btree.h"

#include <cmath>
#include <iostream>

#include "../../io/io_handler/io_handler.h"

using namespace std;

double log(double x, double b) { return log(x) / log(b); }

/**
 * @brief Computes the height of a B-tree.
 * Log_(B+1)(N) -> Branching factor is 1 more than the number of keys in a node.
 *
 * @param num_keys The number of keys in the B-tree.
 * @param B Number of keys per node
 * @return The height of the B-tree. (1 indexed)
 */
int computeHeight(int num_keys, int B) { return ceil(log(num_keys, B + 1)); }

/**
 * @brief Calculates the number of nodes required for a B-tree with num_keys.
 *
 * @param num_keys The total number of keys to be stored in the B-tree.
 * @param B Number of keys per node
 * @return The number of nodes required to store the given number of keys.
 */
int numNodes(int num_keys, int B) {
  return ceil(static_cast<double>(num_keys) / B);
}

/**
 * @brief Calculates the number of keys on the previous layer of a B-tree.
 *
 * @param current_keys The number of keys on the current layer.
 * @param B Number of keys per node
 * @return The number of keys on the previous layer.
 */
int numKeysOnPrevLayer(int current_keys, int B) {
  return ceil(static_cast<double>(current_keys) / (B + 1));
}

/**
 * @brief Computes the starting index of a given layer in a B-tree.
 *
 * @param layer The layer for which the starting index is to be calculated. Leaf
 * layer is layer 0.
 * @param num_keys The total number of keys in the B-tree.
 * @param B Number of keys per node
 * @return The starting index of the specified layer.
 */
int layerStartingIndex(int layer, int num_keys, int B) {
  int page_index = 0;
  while (layer > 0) {
    page_index += numNodes(num_keys, B) * B;
    num_keys = numKeysOnPrevLayer(num_keys, B);
    layer--;
  }
  return page_index;
}

void BTree::loadMetadata() {
  int num_pages = this->io_handler->numPagesInFile(this->filename);
  if (num_pages == -1) {
    return;
  }
  // Metadata is in the last page.
  optional<vector<pair<int, int>>> metadata =
      this->io_handler->readPageFromFile(this->filename, num_pages);
  if (!metadata.has_value()) {
    return;
  };
  vector<pair<int, int>> metadata_data = metadata.value();
  this->num_data = metadata_data[this->B_TREE_NUM_DATA_INDEX].second;
  this->height = metadata_data[this->B_TREE_HEIGHT_INDEX].second;

  this->num_leaf_pages = numNodes(this->num_data, this->B);
}

BTree::BTree(string filename, string dbname, shared_ptr<IO> handler,
             bool use_binary_search) {
  this->use_binary_search = use_binary_search;
  this->filename = filename;
  this->io_handler = handler;
  this->B = io_handler->getPageSize() / io_handler->getPairSize();
  this->loadMetadata();
}

vector<pair<int, int>> BTree::constructMetadata() {
  vector<pair<int, int>> metadata = {
      {this->B_TREE_SIZE_INDEX, this->B},
      {this->B_TREE_HEIGHT_INDEX, this->height},
      {this->B_TREE_NUM_DATA_INDEX, this->num_data}};
  return metadata;
}

int BTree::write(const vector<pair<int, int>> &data) {
  if (io_handler->writeToFile(filename, data) == -1) {
    return EXIT_FAILURE;
  }

  this->num_leaf_pages = io_handler->numPagesInFile(filename);
  this->num_data = data.size();
  this->height = computeHeight(this->num_data, this->B);

  if (io_handler->writeToFileAtPage(this->filename,
                                    this->contructInternalNodes(),
                                    this->num_leaf_pages + 1) == -1) {
    return EXIT_FAILURE;
  }
  int total_pages_without_metadata = io_handler->numPagesInFile(this->filename);
  if (io_handler->writeToFileAtPage(this->filename, this->constructMetadata(),
                                    total_pages_without_metadata + 1) == -1) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

vector<pair<int, int>> BTree::contructInternalNodes() {
  // Number of elements would be the first element in the next layer.
  int total_btree_size =
      layerStartingIndex(this->height, this->num_data, this->B);
  int internal_btree_size = total_btree_size - (this->num_leaf_pages * this->B);

  vector<pair<int, int>> internal_nodes(internal_btree_size,
                                        {this->INF, this->INF});

  for (int height = 1; height < this->height; height++) {
    int next_layer_start =
        layerStartingIndex(height + 1, this->num_data, this->B);
    int this_layer_start = layerStartingIndex(height, this->num_data, this->B);
    int elements_in_this_layer = next_layer_start - this_layer_start;

    for (int element_index = 0; element_index < elements_in_this_layer;
         element_index++) {
      int page_index = element_index / (this->B);  // 0 Indexed
      int page_element_index =
          element_index - (page_index * this->B);  // 0 indexed.
      int right_child_page =
          page_index * (this->B + 1) + page_element_index + 1;

      // Keep calculating until we reach the leaf node.
      for (int i = 0; i < height - 1; i++) {
        right_child_page = right_child_page * (this->B + 1);
      }

      // Since we dont store the leaf nodes, we need to shift backwards by the
      // number of leaf nodes.
      int layer_one_start = layerStartingIndex(1, this->num_data, this->B);
      if (right_child_page * this->B > this->num_data) {
        internal_nodes[this_layer_start + element_index - layer_one_start] = {
            this->INF, this->INF};
        continue;
      }

      int child_page_number = right_child_page + 1;  // 1 indexed

      // TODO: Get from buffer first.
      optional<vector<pair<int, int>>> leaf_page =
          this->io_handler->readPageFromFile(this->filename, child_page_number);
      if (!leaf_page.has_value()) {
        // Handle error
      }
      vector<pair<int, int>> leaf_data = leaf_page.value();

      internal_nodes[this_layer_start + element_index - layer_one_start] =
          leaf_data[0];  // Always the first element.
    }
  }
  return internal_nodes;
}

int BTree::searchInPage(int key, int page_location) {
  int page_number =
      floor(static_cast<double>(page_location) / this->B) + 1;  // 1 indexed
  optional<vector<pair<int, int>>> page =
      this->io_handler->readPageFromFile(this->filename, page_number);
  if (!page.has_value()) {
    // Handle error
  }
  vector<pair<int, int>> data = page.value();
  for (int i = 0; i < this->B; i++) {
    if (key <= data[i].first) {
      return i;
    }
  }
  return this->B;  // If we get here, it means we need the first element of the
                   // next page.
};

optional<pair<int, int>> BTree::getItem(int key, int page_location) {
  int page_number =
      floor(static_cast<double>(page_location) / this->B) + 1;  // 1 indexed
  optional<vector<pair<int, int>>> page =
      this->io_handler->readPageFromFile(this->filename, page_number);
  if (!page.has_value()) {
    return nullopt;
  }
  vector<pair<int, int>> data = page.value();
  for (int i = 0; i < this->B; i++) {
    if (data[i].first == key) {
      return make_pair(data[i].first, data[i].second);
    }
  }
  return nullopt;
}

int BTree::getCloestIndex(int key) {
  // Need to ensure that member variables are populated before calling (or get
  // them now)
  int page_index = 0;
  for (int h = this->height - 1; h > 0; h--) {
    int current_level_location = layerStartingIndex(h, this->num_data, this->B);
    int corresponding_key =
        searchInPage(key, current_level_location + page_index);
    page_index = page_index * (this->B + 1) + corresponding_key * this->B;
  };

  int leaf_page = layerStartingIndex(0, this->num_data, this->B) + page_index;
  int index_in_leaf_node = searchInPage(key, leaf_page);
  return index_in_leaf_node + page_index;
}

optional<int> BTree::btreeGet(int key) {
  int index = getCloestIndex(key);
  optional<pair<int, int>> result = getItem(key, index);
  if (result.has_value()) {
    return result.value().second;
  }
  return nullopt;
};

// BINARY SEARCH METHODS
optional<int> BTree::binarySearchGet(int key) {
  // Find the page which contains the key.
  optional<pair<vector<pair<int, int>>, int>> page_and_num =
      findPageWithValueEqualToOrGraterThan(key);
  if (page_and_num == nullopt) {
    return nullopt;
  }
  vector<pair<int, int>> page = page_and_num.value().first;
  optional<pair<pair<int, int>, int>> result =
      FindEqualToOrNextLargestInPage(key, page);
  if (!result) {
    return nullopt;
  }
  pair<int, int> key_value = result.value().first;
  // If the key result is the key we are looking for, return the value.
  if (key_value.first == key) {
    return key_value.second;
  }
  return nullopt;
}

// Finds a page which contains the key, or the next largest value than the key.
// Returns the page, and the page number if found, or nullopt if not found.
optional<pair<vector<pair<int, int>>, int>>
BTree::findPageWithValueEqualToOrGraterThan(int key) {
  int num_pages = this->num_leaf_pages;
  // Pages are 1 indexed
  int start = 1;
  int end = num_pages;

  // Try to find key can be within the page.
  vector<pair<int, int>> pairs;
  while (start <= end) {
    int mid = start + (end - start) / 2;
    optional<vector<pair<int, int>>> page =
        io_handler->readPageFromFile(filename, mid);

    // Error while reading page.
    if (!page) {
      return nullopt;
    }

    pairs = page.value();

    // If first_element <= key <= last_element, then the key must be in this
    // page if it exists. Otherwise, the next largest value is in this page.
    if (pairs[0].first <= key && key <= pairs[pairs.size() - 1].first) {
      return make_pair(pairs, mid);
    }

    // If the key is larger than the last value, then we shift our search space
    // right
    else if (pairs[pairs.size() - 1].first < key) {
      start = mid + 1;
    }
    // If they key is smaller than the first value, then we shift our search
    // space to the left
    else if (key < pairs[0].first) {
      end = mid - 1;
    }
  }
  // At this point, if we have not found a matching page,
  // Then start contains the next largest value.

  // Check if start is a valid page number.
  if (start > num_pages) {
    return nullopt;
  }

  // Need to read the next largest page to return.
  pairs = io_handler->readPageFromFile(filename, start).value();
  return make_pair(pairs, start);
}

optional<pair<pair<int, int>, int>> BTree::FindEqualToOrNextLargestInPage(
    int key, vector<pair<int, int>> page) {
  int low = 0;
  int high = page.size() - 1;

  while (low <= high) {
    int mid = low + (high - low) / 2;
    if (page[mid].first == key) {
      return make_pair(page[mid], mid);
    } else if (page[mid].first < key) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }
  // If we reach here, the key is at low
  // unless low is out of bounds
  if (low >= page.size()) {
    return nullopt;
  }
  return make_pair(page[low], low);
}

// METHODS
optional<int> BTree::get(int key) {
  if (this->use_binary_search) {
    return binarySearchGet(key);
  }
  return btreeGet(key);
}


vector<pair<int, int>> BTree::scan(int rangeStart, int rangeEnd) {
  vector<pair<int, int>> page_values;
  int index;
  int page_num;

  // Populate variables using binary search
  if (this->use_binary_search) {
    // Find the page which contains the rangeStart, or the next largest value.
    optional<pair<vector<pair<int, int>>, int>> page_and_num =
        findPageWithValueEqualToOrGraterThan(rangeStart);
    if (page_and_num == nullopt) {
      return {};
    }
    page_values = page_and_num.value().first;
    page_num = page_and_num.value().second;

    optional<pair<pair<int, int>, int>> result =
        FindEqualToOrNextLargestInPage(rangeStart, page_values);
    if (result == nullopt) {
      return {};
    }

    pair<int, int> key_value = result.value().first;
    index = result.value().second;
  }
  // Populate variables using B-tree search
  else {
    int index_in_btree = getCloestIndex(rangeStart);
    page_num = (index_in_btree / this->B) + 1;  // 1 indexed
    index = index_in_btree % this->B;
    page_values = io_handler->readPageFromFile(filename, page_num).value();
  }
  vector<pair<int, int>> res;
  int total_pages = io_handler->numPagesInFile(filename);
  while (page_values[index].first <= rangeEnd) {
    res.push_back(page_values[index]);
    index++;

    if (index >= page_values.size()) {
      page_num++;
      if (page_num > total_pages) {
        break;
      }
      page_values = io_handler->readPageFromFile(filename, page_num).value();
      index = 0;
    }
  }

  return res;
};
