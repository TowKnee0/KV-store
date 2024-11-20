#include "sorted_list.h"

#include <iostream>

#include "../../io/io_handler/io_handler.h"

using namespace std;

SortedList::SortedList(string filename, string dbname, shared_ptr<IO> handler)
{
  this->filename = filename;
  this->dbname = dbname;
  this->io_handler = handler;
}

int SortedList::write(const vector<pair<int, int>> &data)
{
  if (io_handler->writeToFile(filename, data) == -1)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

optional<int> SortedList::get(int key)
{
  vector<int> key_data = getKeyData(key);

  // If the key result is the key we are looking for, return the value.
  if (key_data[2] == key)
  {
    return key_data[3];
  }

  return nullopt;
}

// Finds a page which contains the key, or the next largest value than the key.
// Returns the page, and the page number if found, or nullopt if not found.
optional<pair<vector<pair<int, int>>, int>>
SortedList::findPageWithValueEqualToOrGraterThan(int key)
{
  int num_pages = io_handler->numPagesInFile(filename);

  // Pages are 1 indexed
  int start = 1;
  int end = num_pages;

  // Try to find key can be within the page.
  vector<pair<int, int>> pairs;
  while (start <= end)
  {
    int mid = start + (end - start) / 2;
    optional<vector<pair<int, int>>> page =
        io_handler->readPageFromFile(filename, mid);

    // Error while reading page.
    if (!page)
    {
      return nullopt;
    }

    pairs = page.value();

    // If first_element <= key <= last_element, then the key must be in this
    // page if it exists. Otherwise, the next largest value is in this page.
    if (pairs[0].first <= key && key <= pairs[pairs.size() - 1].first)
    {
      return make_pair(pairs, mid);
    }

    // If the key is larger than the last value, then we shift our search space
    // right
    else if (pairs[pairs.size() - 1].first < key)
    {
      start = mid + 1;
    }
    // If they key is smaller than the first value, then we shift our search
    // space to the left
    else if (key < pairs[0].first)
    {
      end = mid - 1;
    }
  }
  // At this point, if we have not found a matching page,
  // Then start contains the next largest value.

  // Check if start is a valid page number.
  if (start > num_pages)
  {
    return nullopt;
  }

  // Need to read the next largest page to return.
  pairs = io_handler->readPageFromFile(filename, start).value();
  return make_pair(pairs, start);
}

// Given a page number, finds the key-value pair which is equal to the key or
// the the next largest value. Returns a pair of <<key, value>, page_index> if
// found, or nullopt if not found.
optional<pair<pair<int, int>, int>> SortedList::FindEqualToOrNextLargestInPage(
    int key, vector<pair<int, int>> page)
{
  int low = 0;
  int high = page.size() - 1;

  while (low <= high)
  {
    int mid = low + (high - low) / 2;
    if (page[mid].first == key)
    {
      return make_pair(page[mid], mid);
    }
    else if (page[mid].first < key)
    {
      low = mid + 1;
    }
    else
    {
      high = mid - 1;
    }
  }
  // If we reach here, the key is at low
  // unless low is out of bounds
  if (low >= page.size())
  {
    return nullopt;
  }
  return make_pair(page[low], low);
}

vector<int> SortedList::getKeyData(int key)
{
  optional<pair<vector<pair<int, int>>, int>> page_and_num =
      findPageWithValueEqualToOrGraterThan(key);
  if (page_and_num == nullopt)
  {
    return {-1, -1, -1, -1};
  }
  vector<pair<int, int>> page = page_and_num.value().first;
  optional<pair<pair<int, int>, int>> result =
      FindEqualToOrNextLargestInPage(key, page);
  if (!result)
  {
    return {-1, -1, -1, -1};
  }
  pair<int, int> key_value = result.value().first;
  return {page_and_num.value().second, result.value().second, key_value.first, key_value.second};
}