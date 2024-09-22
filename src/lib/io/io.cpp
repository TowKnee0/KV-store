#include "io.h"
#include<fstream>
#include <vector>
#include <utility>
#include <cstring> 
#include <iostream>
#include <filesystem>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

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
const int MAX_PAIRS = PAGE_SIZE / PAIR_SIZE; // Maximum amount of pairs that can be stored in a page.

// Helper function
string constructFilePath(const string dbname, const string filename) {
    return "../data/" + dbname + "/" + filename; // Construct the relative path.
}

int writeToFile(const string dbname, const string filename, const vector<pair<int, int>>& data) {
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
            return -1;
    }

    // Allocate memory that is aligned to PAGE_SIZE for Direct I/O.
    void *alignedMemory;
    if (posix_memalign(&alignedMemory, PAGE_SIZE, PAGE_SIZE) != 0) {
        close(fd);
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
            memcpy(static_cast<char*>(alignedMemory) + offset, &current_pair, PAIR_SIZE);
        }

        /**
         * Which page are we on: current_idx / MAX_PAIRS
         * Which byte address is this page aligned to: (current_idx / MAX_PAIRS) * PAGE_SIZE
         */
        // Write at the start of the page
        int pageAddress = (current_idx / MAX_PAIRS) * PAGE_SIZE;
        if (pwrite(fd, alignedMemory, PAGE_SIZE, pageAddress) < 0) {
            free(alignedMemory);
            close(fd);
            remove(fullPath.c_str());
            return -1;
        }

        // Move to the next set of data
        current_idx += pairs_in_this_page;
    }

    free(alignedMemory);
    close(fd);
    return 1; 
}

optional<vector<pair<int, int>>> readPageFromFile(const string dbname, const string filename, int pageNumber) {
    string fullPath = constructFilePath(dbname, filename);
    ifstream in(fullPath, ios_base::binary);

    // pageNumber is 1-indexed
    size_t offset = (pageNumber - 1) * PAGE_SIZE;

    in.seekg(offset);

    // Check if the seek was unsuccessful.
    if (in.fail()) {
        return nullopt; 
    }

    // Create a buffer to read the entire page
    char buffer[PAGE_SIZE];
    in.read(buffer, PAGE_SIZE);


    vector<pair<int, int>> pairs;

    // Calculate how many pairs we need to process based on the count of bytes read
    streamsize bytesRead = in.gcount();
    int pairsRead = bytesRead / PAIR_SIZE;

    // Process the buffer to extract pairs
    for (int i = 0; i < pairsRead; ++i) {
        pair<int, int> current_pair;

        // Calculate the offset in the buffer
        int pairOffset = i * PAIR_SIZE;

        // Copy the key and value from the buffer into the current_pair
        if (buffer[pairOffset] != '\0' && buffer[pairOffset + sizeof(int)] != '\0') {
            memcpy(&current_pair.first, &buffer[pairOffset], sizeof(int));
            memcpy(&current_pair.second, &buffer[pairOffset + sizeof(int)], sizeof(int));
            pairs.push_back(current_pair);
        }
    }

    in.close();
    return pairs; // Return the read key-value pairs.
}

// TODO: Delete this
int main() {
    // Sample database and filename
    string dbname = "test_db";
    string filename = "datafile.bin";

    // Generate a large dataset to write
    vector<pair<int, int>> data;
    for (int i = 1; i <= 1; ++i) {
        pair<int, int> newPair = {i, i + 1};
        data.push_back(newPair);
    }

    cout << "Starting the write operation..." << endl;

    // Write the sample data to the file
    int writeResult = writeToFile(dbname, filename, data);
    if (writeResult == -1) {
        cout << "Error writing to file." << endl;
        return 1;
    }
    cout << "Successfully wrote " << data.size() << " pairs to the file." << endl;

    for (int pageNum = 1; pageNum <= 5; pageNum++) {
        cout << "Attempting to read page " << pageNum << " from the file..." << endl;
        optional<vector<pair<int, int>>> readData = readPageFromFile(dbname, filename, pageNum);
        if (!readData) {
            cout << "Error reading from page " << pageNum << " of the file." << endl;
            continue;
        }

        // Output the read data
        cout << "Reading page " << pageNum << " with an observed size of " << readData->size() << endl;
        for (const auto& pair : *readData) {
            cout << "Key: " << pair.first << ", Value: " << pair.second << endl;
        }
        cout << "Finished reading from page " << pageNum << "." << endl;
    }

    return 0;
}