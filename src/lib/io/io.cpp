#include "io.h"
#include <fstream>
#include <vector>
#include <utility>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <climits>

using namespace std;

/**
 * When we write data to a file we will fill it in 4KB pages.
 * Note that key and value are 4-byte integers each, so a key value pair will take up 8-bytes.
 * This suggests each 4KB or 4096byte page can hold up to 4096 / 8 = 512 key value pairs.
 * If we use plain text we will use up extra space for the delimiters. And the performance is worse anyway.
 * We will write the information in a binary format to be more performant.
 */
const int PAGE_SIZE = 4096; // 4KB page size.
const int PAIR_SIZE = sizeof(int) * 2; // Size of one key-value pair (2 integers).
const int MAX_PAIRS = PAGE_SIZE / PAIR_SIZE; // Maximum number of pairs that can be stored in a page.
const int TOMBSTONE_VALUE = INT_MIN; // Special value to indicate an empty key / value.

IOHandler::IOHandler(const string dbname) : dbname(dbname) {}

// Helper function
string constructFilePath(const string dbname, const string filename){
    return "../data/" + dbname + "/" + filename; // Construct the relative path.
}

int IOHandler::writeToFile(const string filename, const vector<pair<int, int>>& data) {
     // Create the directory if it doesn't exist.
    string fullPath = constructFilePath(dbname, filename);
    filesystem::path dirPath = filesystem::path(fullPath).parent_path();
    if (!filesystem::exists(dirPath)) {
        filesystem::create_directories(dirPath);
    }
    
    int data_size = data.size(); 
    
    // Create the file if it doesn't exist already and prepare it for Direct I/O + Write Only Access.
    int fd = open(fullPath.c_str(), O_CREAT | O_DIRECT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        remove(fullPath.c_str());
        cout << "fd < 0" << endl;
        return -1;
    }

    // Allocate memory that is aligned to PAGE_SIZE for Direct I/O.
    void *alignedMemory;
    if (posix_memalign(&alignedMemory, PAGE_SIZE, PAGE_SIZE) != 0)
    {
        close(fd);
        cout << "posix_memalign != 0" << endl;
        return -1;
    }

    // Represents how many pairs we have processed so far
    int current_idx = 0;

    // Write 1 page at a time to disk
    while (current_idx < data_size) {
        memset(alignedMemory, '\0', PAGE_SIZE);
        int remaining_pairs = data_size - current_idx;
        int pairs_in_this_page = (remaining_pairs < MAX_PAIRS) ? remaining_pairs : MAX_PAIRS;
        int bytes_to_write = pairs_in_this_page * PAIR_SIZE;

        // Put the current pair we are looking at inside the aligned memory at the correct position
        // We reserved PAIR_SIZE bytes for each pair. So when writing data for a new pair within a page we need to write at an offset of i * PAIR_SIZE.
        for (int i = 0; i < pairs_in_this_page; i++) {
            const pair<int, int> current_pair = data[current_idx + i];
            int offset = i * PAIR_SIZE;
            memcpy(static_cast<char *>(alignedMemory) + offset, &current_pair, PAIR_SIZE);
        }

        // When there are pairs present than the maximum capacity of the page we will it with ignore values
        if (pairs_in_this_page < MAX_PAIRS)
        {
            for (int i = pairs_in_this_page; i < MAX_PAIRS; i++)
            {
                pair<int, int> tombstone = {TOMBSTONE_VALUE, TOMBSTONE_VALUE};
                int offset = i * PAIR_SIZE;
                memcpy(static_cast<char *>(alignedMemory) + offset, &tombstone, PAIR_SIZE);
            }
        }

        /**
         * Which page are we on: current_idx / MAX_PAIRS
         * Which byte address is this page aligned to: (current_idx / MAX_PAIRS) * PAGE_SIZE
         */
        // Write at the start of the page
        int pageAddress = (current_idx / MAX_PAIRS) * PAGE_SIZE;
        if (pwrite(fd, alignedMemory, PAGE_SIZE, pageAddress) < 0)
        {
            free(alignedMemory);
            close(fd);
            remove(fullPath.c_str());
            cout << "prwite  < 0" << endl;
            return -1;
        }

        // Move to the next set of data
        current_idx += pairs_in_this_page;
    }

    free(alignedMemory);
    close(fd);
    return 1;
}

optional<vector<pair<int, int>>> IOHandler::readPageFromFile(const string filename, int pageNumber) {
    // Calculate the memory address for the page
    size_t pageMemoryAddress = (pageNumber - 1) * PAGE_SIZE;
    string fullPath = constructFilePath(dbname, filename);
    int fd = open(fullPath.c_str(), O_DIRECT | O_RDONLY);
    
    if (fd < 0) {
        return nullopt; // Handle error (file doesn't exist or can't be opened)
    }

    // Allocate an aligned buffer for Direct I/O
    void *buffer;
    if (posix_memalign(&buffer, PAGE_SIZE, PAGE_SIZE) != 0)
    {
        close(fd);
        return nullopt; // Handle allocation error
    }

    // Read the page into the aligned buffer and handle error accordingly
    int bytesRead = pread(fd, buffer, PAGE_SIZE, pageMemoryAddress);
    int pairsRead = bytesRead / PAIR_SIZE;
    if (bytesRead < 0)
    {
        free(buffer);
        close(fd);
        return nullopt; // Handle read error
    }

    vector<pair<int, int>> pairs;

    // Extract non-tombstone pairs
    for (int i = 0; i < pairsRead; ++i)
    {
        // Calculate the offset in the buffer
        int pairOffset = i * PAIR_SIZE;

        // Careful pointer arithmetic to get the memory address of the key / value for the current pair
        char *keyPtr = static_cast<char *>(buffer) + pairOffset;
        char *valuePtr = static_cast<char *>(buffer) + pairOffset + sizeof(int);

        // Skip tombstone values (dereference before comparison)
        if (*(int *)keyPtr == TOMBSTONE_VALUE && *(int *)valuePtr == TOMBSTONE_VALUE)
        {
            continue;
        }

        // Copy the key and value into the current_pair
        pair<int, int> current_pair;
        memcpy(&current_pair.first, keyPtr, sizeof(int));
        memcpy(&current_pair.second, valuePtr, sizeof(int));
        pairs.push_back(current_pair);
    }

    free(buffer);
    close(fd);
    return pairs; // Return the read key-value pairs.
}

vector<string> IOHandler::getDbFilenames(){ return {};};
int IOHandler::numPagesInFile(const string filename){ return -1;};