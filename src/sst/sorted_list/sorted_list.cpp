#include "sorted_list.h"

#include <iostream>

#include "../../io/io_handler/io_handler.h"

using namespace std;

SortedList::SortedList(string filename, string dbname, shared_ptr<IO> handler) {
  this->filename = filename;
  this->dbname = dbname;
  this->io_handler = handler;
}

int SortedList::write(const vector<pair<int, int>> &data) {
  if (io_handler->writeToFile(filename, data) == -1) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

optional<int> SortedList::get(int key) {
  // Find the page which contains the key.
  optional<int> page = findPageWithValueEqualToOrGraterThan(key);
  if (!page) {
    return nullopt;
  }

  // Find the key in the page. Technically this does 2 ios for the same page, but saves
  // us from having to write two helper functions.
  optional<pair<pair<int, int>, int>> result = FindEqualToOrNextLargestInPage(key, page.value());
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

vector<pair<int, int>> SortedList::scan(int rangeStart, int rangeEnd) {

  // Find the page which contains the rangeStart, or the next largest value.
  optional<int> page = findPageWithValueEqualToOrGraterThan(rangeStart);
  if (!page) {
    return {};
  }
  // Find where the rangeStart is in the page.
  int page_num = page.value();
  optional<pair<pair<int, int>, int>> result = FindEqualToOrNextLargestInPage(rangeStart, page_num);
  if (!result) {
    return {};
  }

  pair<int, int> key_value = result.value().first;
  int index = result.value().second;
  
  // This does 2 ios for the first page, as we find the find the starting index, then read it again
  // for the rest of the values.
  vector<pair<int, int>> page_values = io_handler->readPageFromFile(filename, page_num).value();
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

}

// Finds a page which contains the key, or the next largest value than the key.
// Returns the page number if found, or nullopt if not found.
optional<int> SortedList::findPageWithValueEqualToOrGraterThan(int key) {
  int num_pages = io_handler->numPagesInFile(filename);

  // Pages are 1 indexed
  int start = 1;
  int end = num_pages;

  // Try to find key can be within the page.
  while (start <= end) {
    int mid = start + (end - start) / 2;
    optional<vector<pair<int, int>>> page =
        io_handler->readPageFromFile(filename, mid);

    // Error while reading page.
    if (!page) {
      return -1;
    }

    vector<pair<int, int>> pairs = page.value();

    // If first_element <= key <= last_element, then the key must be in this
    // page if it exists. Otherwise, the next largest value is in this page.
    if (pairs[0].first <= key && key <= pairs[pairs.size() - 1].first) {
      return mid;
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
  return start;
}

// Given a page number, finds the key-value pair which is equal to the key or the
// the next largest value.
// Returns a pair of <<key, value>, page_index> if found, or nullopt if not found.
optional<pair<pair<int, int>, int>> SortedList::FindEqualToOrNextLargestInPage(int key, int page){
  
  optional<vector<pair<int, int>>> page_data = io_handler->readPageFromFile(filename, page);
  if (!page_data) {
    return nullopt;
  }

  vector<pair<int, int>> pairs = page_data.value();
  int low = 0;
  int high = pairs.size() - 1;

  while (low <= high) {
    int mid = low + (high - low) / 2;
    if (pairs[mid].first == key) {
      return make_pair(pairs[mid], mid);
    } else if (pairs[mid].first < key) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }
  // If we reach here, the key is at low
  // unless low is out of bounds
  if (low >= pairs.size()) {
    return nullopt;
  }
  return make_pair(pairs[low], low);
}