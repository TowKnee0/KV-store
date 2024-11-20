#include "btree.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

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

/**
 * @brief Loads the metadata for the B-tree from the last file in the page.
 *
 * If the file is not found (no data yet) returns early.
 *
 */
void BTree::loadMetadata() {
  int num_pages = this->io_handler->numPagesInFile(this->filename);
  if (num_pages == -1) {
    return;
  }
  // Metadata is in the last page.
  optional<vector<pair<int, int>>> metadata =
      this->buffer->getPage(this->filename, num_pages);
  if (!metadata.has_value()) {
    return;
  };
  vector<pair<int, int>> metadata_data = metadata.value();
  this->num_data = metadata_data[BTree::B_TREE_NUM_DATA_INDEX].second;
  this->height = metadata_data[BTree::B_TREE_HEIGHT_INDEX].second;

  this->num_leaf_pages = numNodes(this->num_data, this->B);
}

/**
 * @brief Constructs a BTree object.
 *
 * @param filename The name of the file where the BTree data is stored.
 * @param dbname The name of the database.
 * @param handler A shared pointer to an IO handler.
 * @param buffer A shared pointer to a Buffer.
 * @param use_binary_search whether to use binary search.
 */
BTree::BTree(string filename, string dbname, shared_ptr<IO> handler,
             shared_ptr<Buffer> buffer, bool use_binary_search, shared_ptr<Filter> filter) {
  this->use_binary_search = use_binary_search;
  this->filename = filename;
  this->io_handler = handler;
  this->buffer = buffer;
  this->filter = filter;
  this->B = io_handler->getPageSize() / io_handler->getPairSize();
  this->loadMetadata();
}

/**
 * @brief Constructs metadata for the BTree.
 *
 * Wirtes the branching factor, tree height, and number of leaf nodes to a
 * vector.
 * @return A vector of pairs containing the metadata of the BTree.
 */
vector<pair<int, int>> BTree::constructMetadata() {
  vector<pair<int, int>> metadata = {
      {BTree::B_TREE_SIZE_INDEX, this->B},
      {BTree::B_TREE_HEIGHT_INDEX, this->height},
      {BTree::B_TREE_NUM_DATA_INDEX, this->num_data}};
  return metadata;
}

/**
 * @brief Writes the given data to the B-tree structure and updates the
 * metadata.
 *
 * @param data A vector of key-value pairs to be written to the B-tree.
 * @return int Returns EXIT_SUCCESS on successful write, otherwise returns
 * EXIT_FAILURE.
 */
int BTree::write(const vector<pair<int, int>> &data) {
  // Write leafs
  if (io_handler->writeToFile(filename, data) == -1) {
    return EXIT_FAILURE;
  }

  cout << "Btree write 1" << endl;
  // If we have a filter, add all the keys to the filter.
  // Part 3 only.
  if (this->filter != nullptr) {
    for (auto &pair : data) {
      this->filter->addKey(pair.first);
    }
  }

  cout << "Btree write 2" << endl;


  this->num_leaf_pages =
      ceil(static_cast<double>(data.size()) /
           this->B);  // io_handler->numPagesInFile(filename);
  this->num_data = data.size();
  this->height = computeHeight(this->num_data, this->B);

  cout << "Btree write 3" << endl;


  if (io_handler->writeToFile(this->filename, this->contructInternalNodes(),
                              this->num_leaf_pages + 1) == -1) {
    return EXIT_FAILURE;
  }

  cout << "Btree write 4" << endl;


  int total_pages_without_metadata =
      ceil(static_cast<double>(
               layerStartingIndex(this->height, data.size(), this->B)) /
           this->B);  // io_handler->numPagesInFile(this->filename);

  cout << "Btree write 5" << endl;

  if (io_handler->writeToFile(this->filename, this->constructMetadata(),
                              total_pages_without_metadata + 1) == -1) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void incrementPointersAndData(int &page_pointer, int &page,
                              vector<pair<int, int>> &data,
                              shared_ptr<Buffer> buffer, string filename,
                              int B) {
  page_pointer++;
  if (page_pointer == B) {
    page++;
    page_pointer = 0;
    data = buffer->getPage(filename, page).value();
  }
}

void addDataToBuffer(vector<pair<int, int>> &page_buffer,
                     vector<pair<int, int>> &page, int page_pointer, Filter &filter, int &num_data) {
  page_buffer.push_back(page[page_pointer]);
  // Merge should only be used in part 3, and therefore a filter should
  // always be present.
  filter.addKey(page[page_pointer].first);
  num_data++;
}

void checkAndFlushPageBuffer(vector<pair<int, int>> &page_buffer,
                             int &new_num_leaf_pages, int B, string filename,
                             shared_ptr<IO> io_handler) {
  if (page_buffer.size() == B) {
    if (io_handler->writeToFile(filename, page_buffer,
                                new_num_leaf_pages + 1) == -1) {
      throw runtime_error("Error writing to file");
    }
    page_buffer.clear();
    new_num_leaf_pages++;
  }
}

/**
 * @brief Merges two B-trees from the specified files. Assumes the second
 * filename corresponds to the newer tree.
 *
 * @param filename1 The name of the first file containing a B-tree.
 * @param filename2 The name of the second file containing a B-tree. This is the
 * NEWER TREE.
 * @return returns EXIT_FAILURE or EXIT_SUCCESS.
 */
int BTree::mergeBTrees(string filename1, string filename2) {
  
  // --- Metadata fetching ---
  int num_pages1 = io_handler->numPagesInFile(filename1);
  int num_pages2 = io_handler->numPagesInFile(filename2);
  if (num_pages1 == -1 || num_pages2 == -1) {
    return EXIT_FAILURE;
  }
  // Read the metadata of the two files.
  optional<vector<pair<int, int>>> metadata1 =
      this->buffer->getPage(filename1, num_pages1);
  optional<vector<pair<int, int>>> metadata2 =
      this->buffer->getPage(filename2, num_pages2);
  if (!metadata1.has_value() || !metadata2.has_value()) {
    return EXIT_FAILURE;
  }

  vector<pair<int, int>> metadata_data1 = metadata1.value();
  vector<pair<int, int>> metadata_data2 = metadata2.value();

  int num_data_tree1 = metadata_data1[this->B_TREE_NUM_DATA_INDEX].second;
  int num_data_tree2 = metadata_data2[this->B_TREE_NUM_DATA_INDEX].second;

  int B_tree_1 = metadata_data1[this->B_TREE_SIZE_INDEX].second;
  int B_tree_2 = metadata_data2[this->B_TREE_SIZE_INDEX].second;

  int num_leaf_pages_tree1 = numNodes(num_data_tree1, B_tree_1);
  int num_leaf_pages_tree2 = numNodes(num_data_tree2, B_tree_2);

  int tree1_page = 1;
  int tree1_page_pointer = 0;
  int tree2_page = 1;
  int tree2_page_pointer = 0;
  vector<pair<int, int>> tree1_data =
      this->buffer->getPage(filename1, tree1_page).value();

  vector<pair<int, int>> tree2_data =
      this->buffer->getPage(filename2, tree2_page).value();
  vector<pair<int, int>> page_buffer;
  int new_num_leaf_pages = 0;
  int new_num_data = 0;

  // --- Merging ---

  while (tree1_page <= num_leaf_pages_tree1 &&
         tree1_page <= num_leaf_pages_tree2) {
    if (tree1_data[tree1_page_pointer].first == this->INF) {
      tree1_page =
          num_leaf_pages_tree1 + 1;  // Inf is used to pad the end of the page,
      // so we can skip the rest of the data here. Should only occur at the last
      // page.
      break;
    }
    if (tree2_data[tree2_page_pointer].first == this->INF) {
      tree2_page = num_leaf_pages_tree2 + 1;  // same as above
      break;
    }

    if (tree1_data[tree1_page_pointer].first <
        tree2_data[tree2_page_pointer].first) {
      addDataToBuffer(page_buffer, tree1_data, tree1_page_pointer, *this->filter, new_num_data);
      incrementPointersAndData(tree1_page_pointer, tree1_page, tree1_data,
                               this->buffer, filename1, this->B);
    } else {
      bool same_key = tree1_data[tree1_page_pointer].first ==
                      tree2_data[tree2_page_pointer].first;
      addDataToBuffer(page_buffer, tree2_data, tree2_page_pointer, *this->filter, new_num_data);
      incrementPointersAndData(tree2_page_pointer, tree2_page, tree2_data,
                               this->buffer, filename2, this->B);
      if (same_key) {
        incrementPointersAndData(tree1_page_pointer, tree1_page, tree1_data,
                                 this->buffer, filename1, this->B);
      }
    }
    checkAndFlushPageBuffer(page_buffer, new_num_leaf_pages, this->B,
                            this->filename, this->io_handler);
  }

  // At this point, either both trees have been finished or one of them has been
  // finished.
  while (tree1_page <= num_leaf_pages_tree1) {
    if (tree1_data[tree1_page_pointer].first == this->INF) {
      break;  // No more useful data.
    }
    addDataToBuffer(page_buffer, tree1_data, tree1_page_pointer, *this->filter, new_num_data);
    incrementPointersAndData(tree1_page_pointer, tree1_page, tree1_data,
                             this->buffer, filename1, this->B);
    checkAndFlushPageBuffer(page_buffer, new_num_leaf_pages, this->B,
                            this->filename, this->io_handler);
  }

  while (tree2_page <= num_leaf_pages_tree2) {
    if (tree2_data[tree2_page_pointer].first == this->INF) {
      break;  // No more useful data.
    }
    addDataToBuffer(page_buffer, tree2_data, tree2_page_pointer, *this->filter, new_num_data);
    incrementPointersAndData(tree2_page_pointer, tree2_page, tree2_data,
                             this->buffer, filename2, this->B);
    checkAndFlushPageBuffer(page_buffer, new_num_leaf_pages, this->B,
                            this->filename, this->io_handler);
  }
  // Flush whatever is left in the buffer
  if (page_buffer.size() > 0) {
    if (io_handler->writeToFile(this->filename, page_buffer,
                                new_num_leaf_pages + 1) == -1) {
      throw runtime_error("Error writing to file");
    }
    new_num_leaf_pages++;
  }

  // Set the new metadata;
  this->num_data = new_num_data;
  this->num_leaf_pages = new_num_leaf_pages;
  this->height = computeHeight(this->num_data, this->B);
  // construct internal nodes
  if (io_handler->writeToFile(this->filename, this->contructInternalNodes(),
                              this->num_leaf_pages + 1) == -1) {
    return EXIT_FAILURE;
  }

  // Write the metadata
  int total_pages_without_metadata =
      ceil(static_cast<double>(
               layerStartingIndex(this->height, new_num_data, this->B)) /
           this->B);
  if (io_handler->writeToFile(this->filename, this->constructMetadata(),
                              total_pages_without_metadata + 1) == -1) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/**
 * @brief Constructs the internal nodes of the B-Tree.
 *
 * @return A vector of pairs representing the internal nodes of the B-Tree.
 */
vector<pair<int, int>> BTree::contructInternalNodes() {
  // Number of elements would be the first element in the next layer.
  // Height is 1 indexed.

  cout << "height: " << this->height << " num_data: " << this->num_data << " num_leaf_pages: " << this->num_leaf_pages << " B: " << this->B << endl;

  int total_btree_size =
      layerStartingIndex(this->height, this->num_data, this->B);

  int internal_btree_size = total_btree_size - (this->num_leaf_pages * this->B);

  vector<pair<int, int>> internal_nodes(internal_btree_size,
                                        {this->INF, this->INF});

  // Build up each layer by traversing downnwards down the tree for each
  // element.
  for (int h = 1; h < this->height; h++) {
    int next_layer_index =
        layerStartingIndex(h + 1, this->num_data, this->B);

    int this_layer_index = layerStartingIndex(h, this->num_data, this->B);
    int elements_in_this_layer = next_layer_index - this_layer_index;

    // For each position in the current layer, find what it's right child should
    // be to populate it.
    for (int layer_element_index = 0;
         layer_element_index < elements_in_this_layer; layer_element_index++) {
      int page_index = layer_element_index / (this->B);  // 0 Indexed

      int element_index_in_page =
          layer_element_index - (page_index * this->B);  // 0 indexed.

      int right_child_page =
          page_index * (this->B + 1) + element_index_in_page + 1;

      // Keep calculating until we reach the leaf node.
      for (int i = 0; i < h - 1; i++) {
        right_child_page = right_child_page * (this->B + 1);
      }

      // Since we dont store the leaf nodes, we need to shift backwards by the
      // number of elements in the leaf nodes.
      int offset = layerStartingIndex(1, this->num_data, this->B);

      // If the right child is out of bounds set it to infinity.
      if (right_child_page * this->B > this->num_data) {
        internal_nodes[this_layer_index + layer_element_index - offset] = {
            BTree::INF, BTree::INF};
        continue;
      }

      int child_page_number = right_child_page + 1;  // add 1 because 1 indexed.

      optional<vector<pair<int, int>>> leaf_page =
          this->buffer->getPage(this->filename, child_page_number);

      if (!leaf_page.has_value()) {
        string error_message = "expecting leaf page to be present at " +
                               to_string(child_page_number);
        throw runtime_error(error_message);
      }
      vector<pair<int, int>> leaf_data = leaf_page.value();

      internal_nodes[this_layer_index + layer_element_index - offset] =
          leaf_data[0];  // Always the first element.
    }
  }
  cout << "Btree constructInternalNodes return" <<endl;

  return internal_nodes;
}

/**
 * @brief Finds which index the given key corresponds to within the page.
 *
 * That is, it finds the first index in which key <= btree[index]
 *
 * @param key The key to find within the page.
 * @param page_location The index where the page starts in the btree.
 * @return A pair of <index, <key, value>>
 */
pair<int, pair<int, int>> BTree::findKeyInPage(int key, int page_location) {
  int page_number =
      floor(static_cast<double>(page_location) / this->B) + 1;  // 1 indexed
  optional<vector<pair<int, int>>> page =
      this->buffer->getPage(this->filename, page_number);
  if (!page.has_value()) {
    string error_message = "expecting to find key: " + to_string(key) +
                           " in page: " + to_string(page_number);
    throw runtime_error(error_message);
  }
  vector<pair<int, int>> data = page.value();
  for (int i = 0; i < this->B; i++) {
    if (key <= data[i].first) {
      return make_pair(i, data[i]);
    }
  }
  return make_pair(
      this->B,
      make_pair(this->INF,
                this->INF));  // If we get here, it means we need the first
                              // element of the next page. INF as a placeholder.
};

/**
 * @brief Get the index of the key in the btree. If the key is not present,
 * it returns the index of the next largest key. (helpful for scanning)
 *
 * @param key The key to find in the BTree.
 * @return int The index of the key or next largest key in the BTree.
 */
int BTree::getIndexInBTree(int key) {
  int page_index = 0;
  // Loop through each layer of the BTree, start at height - 1 since 1 indexed.
  // Stop at 0 since layer 0 is the leaf layer and is handled differently.
  for (int h = this->height - 1; h > 0; h--) {
    int level_index = layerStartingIndex(h, this->num_data, this->B);
    int corresponding_key_index =
        findKeyInPage(key, level_index + page_index).first;
    page_index = page_index * (this->B + 1) + corresponding_key_index * this->B;
  };

  // Page index is now the leaf index
  int index_in_leaf_node = findKeyInPage(key, page_index).first;
  return index_in_leaf_node + page_index;
}

/**
 * @brief Gets the value associated with the given key in the B-tree.
 *
 * Btree implementation since we also include binary search implementation.
 *
 * @param key The key to search for in the B-tree.
 * @return An optional containing the value of the key if found,
 *         otherwise null
 */
optional<int> BTree::btreeGet(int key) {
  int index = getIndexInBTree(key);
  pair<int, pair<int, int>> result = findKeyInPage(key, index);
  // If the key is what we are looking for, return the value.
  // (If the key is not found, it returns the next largest key)
  if (result.second.first == key) {
    return result.second.second;
  }
  return nullopt;
};

// BINARY SEARCH METHODS COPIED FROM SortedList.cpp
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

// SST INTERFACE METHODS
/**
 * @brief Gets the value with the given key in the B-tree.
 *
 * @param key The key to search for in the B-tree.
 * @return An optional integer containing the value associated with the key,
 *         or an empty optional if the key is not found.
 */
optional<int> BTree::get(int key) {
  // If we have a filter present, check the filter first.
  if (this->filter != nullptr) {
    if (!this->filter->hasKey(key)) {
      return nullopt;
    }
  }

  if (this->use_binary_search) {
    return binarySearchGet(key);
  }
  return btreeGet(key);
}

/**
 * @brief Scans the tree for key-value pairs within a specified range.
 *
 * @param rangeStart The starting key of the range to scan.
 * @param rangeEnd The ending key of the range to scan.
 * @return A vector of key-value pairs where the keys are within the specified
 * range.
 */
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
    int index_in_btree = getIndexInBTree(rangeStart);
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

/**
 * Renames the current file associated with the tree
 *
 * @param filename The new name for the file.
 * @throws runtime_error if the rename fails.
 */
void BTree::rename(string filename) {
  int renamed = this->io_handler->renameFile(this->filename, filename);
  if (renamed == -1) {
    string error_msg = "Error renaming file: " + this->filename;
    throw runtime_error(error_msg);
  }
  this->filename = filename;
};