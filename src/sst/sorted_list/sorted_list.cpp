#include "sorted_list.h"
#include "../../io/io_handler/io_handler.h"
#include <iostream>

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
    int num_pages = io_handler->numPagesInFile(filename);
    if (num_pages == -1)
    {
        return nullopt;
    }


    // Pages are 1 indexed
    int start = 1;
    int end = num_pages;

    // Perform binary search on the pages.
    while (start <= end)
    {
        int mid = start + (end - start) / 2;
        optional<vector<pair<int, int>>> page = io_handler->readPageFromFile(filename, mid);

        // Error while reading page.
        if (!page)
        {
            return nullopt;
        }

        vector<pair<int, int>> pairs = page.value();
        

        // Perform binary search on the page
        int low = 0;
        int high = pairs.size() - 1;

        // Before doing binary search in the page, confirm that the key could be in this page.

        // if the key is less than the first key in the page, then it is not in this page, and we
        // should update our binary search scope to search in the bottom half of the pages.
        if (key < pairs[low].first)
        {
            end = mid - 1;
            continue;
        }

        // if the key is greater than the last key in the page, then it is not in this page, and we
        // should update our binary search scope to search in the top half of the pages.
        if (key > pairs[high].first)
        {
            start = mid + 1;
            continue;
        }

        // Otherwise, they key is in this page, or not in this sst file.

        while (low <= high)
        {
            int mid = low + (high - low) / 2;
            if (pairs[mid].first == key)
            {
                return pairs[mid].second;
            }
            else if (pairs[mid].first < key)
            {
                low = mid + 1;
            }
            else
            {
                high = mid - 1;
            }
        }

        // If we reach here, the key is not in this page.
        return nullopt;
    }
    // If we reach here, the key is not in the sst file.
    return nullopt;
}

vector<entry> *SortedList::scan(int rangeStart, int rangeEnd)
{

    // Step 1: Find out how many pages are in the file.
    int num_pages = io_handler->numPagesInFile(filename);
    vector<entry> *result = new vector<entry>();

    // Step 2: perform binary search on the page to find the rangeStart.
    // Once we find the page, we can start scanning from there until we reach rangeEnd.
    // We may need to scan multiple pages to get all the key-value pairs in the range.

    // Pages are 1 indexed
    int start = 1;
    int end = num_pages;

    bool should_continue = false;

    // Perform binary search on the pages.
    while (start <= end)
    {
        int mid = start + (end - start) / 2;
        optional<vector<pair<int, int>>> page = io_handler->readPageFromFile(filename, mid);

        // Error while reading page.
        if (!page)
        {
            return nullptr;
        }

        vector<pair<int, int>> pairs = page.value();

        // Perform binary search on the page
        int low = 0;
        int high = pairs.size() - 1;

        // Before doing binary search in the page, confirm that the key could be in this page.

        // if the key is less than the first key in the page, then it is not in this page, and we
        // should update our binary search scope to search in the bottom half of the pages.
        if (rangeStart < pairs[low].first)
        {
            end = mid - 1;
            continue;
        }

        // if the key is greater than the last key in the page, then it is not in this page, and we
        // should update our binary search scope to search in the top half of the pages.
        if (rangeStart > pairs[high].first)
        {
            start = mid + 1;
            continue;
        }

        // Otherwise, they key is in this page, or not in this sst file.

        // Binary search to find the first key which is larger than rangeStart.
        while (low <= high)
        {
            int mid = low + (high - low) / 2;
            if (pairs[mid].first <= rangeStart)
            {
                low = mid + 1;
            }
            else
            {
                high = mid - 1;
            }
        }

        // At this point low should be the index of the first key which is larger than rangeStart.
        // Continue scanning from this point until we reach the end of the page, or we reach rangeEnd.
        while (low < pairs.size() && pairs[low].first < rangeEnd)
        {
            entry e;
            e.key = pairs[low].first;
            e.value = pairs[low].second;
            result->push_back(e);
            low++;
        }

        // Set the should continue flag to false if we have reached the end of the range.
        if (low < pairs.size() && pairs[low].first >= rangeEnd)
        {
            should_continue = false;
        }
        should_continue = true;
        break;
    }

    start++; // We should have read the first page already so we can start from the next page.
    while (should_continue)
    {
        // Read the next page and continue scanning.
        optional<vector<pair<int, int>>> page = io_handler->readPageFromFile(filename, start);
        if (!page)
        {
            // If we run out of pages, we should stop scanning and return what we have so far.
            return result;
        }

        vector<pair<int, int>> pairs = page.value();

        // Continue scanning from the beginning of the page until we reach rangeEnd.
        int low = 0;
        while (low < pairs.size() && pairs[low].first < rangeEnd)
        {
            entry e;
            e.key = pairs[low].first;
            e.value = pairs[low].second;
            result->push_back(e);
            low++;
        }

        // Set the should continue flag to false if we have reached the end of the range.
        if (low < pairs.size() && pairs[low].first >= rangeEnd)
        {
            should_continue = false;
        }
        start++;
    }

    return result;
}