#include "io.h"
#include<fstream>
#include <vector>
#include <utility>
#include <cstring>

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

int writeToFile(const string& filename, const vector<pair<int, int>>& data) {
    int data_size = data.size(); 
    
    ofstream out(filename, ios_base::binary);
    
    // Handle error opening file.
    if (!out) {
        remove(filename.c_str());
        return -1;
    }

    // Since we need to each page in bulk to minimize I/O operations, we will use a buffer to write the data in multiples of the page size.
    // The buffer will contain the data for the most recent page.
    char buffer[PAGE_SIZE]; 

    // Pointer to keep track of how much of the data has been processed so far.
    int current_idx = 0;

    while (current_idx < data_size) {
        // We are going to process the data for a new page so we need to clear recent buffer which might contain old values.
        memset(buffer, 0, PAGE_SIZE);

        // We need to know whether we can fill this buffer all the way or if we can partially fill it.
        int remaining_pairs = data_size - current_idx;
        int pairs_in_this_page;
        
        // Case 1: Defecit
        if (remaining_pairs < MAX_PAIRS) {
            pairs_in_this_page = remaining_pairs;
        }
        // Case 2: Excess
        else {
            pairs_in_this_page = MAX_PAIRS;
        }

        // Populate the buffer
        for (int i = 0; i < pairs_in_this_page; i++) {
            const pair<int, int> current_pair = data[current_idx + i];

            // We reserved PAIR_SIZE bytes for each pair. So when writing data for a new pair we need to write at an offset of i * PAIR_SIZE.
            int offset = i * PAIR_SIZE;

            // Copy the key & value into the buffer.
            memcpy(buffer + offset, &current_pair.first, sizeof(int));
            memcpy(buffer + offset + sizeof(int), &current_pair.second, sizeof(int));
        }

        current_idx += pairs_in_this_page;

        // Write the entire buffer to the file and handle errors.
        if (!out.write(buffer, PAGE_SIZE)) {
            out.close();
            remove(filename.c_str());
            return -1;
        }
    }

    // Success. Close the file after all pages are written.
    out.close();
    return 1;
}

optional<vector<pair<int, int>>> readPageFromFile(const string filename, int pageNumber) {
    ifstream in(filename, ios_base::binary);
    
    // pageNumber is 1-indexed
    size_t offset = (pageNumber - 1) * PAGE_SIZE;

    in.seekg(offset);
    
     // Check if the seek was successful or if we are at the end of the file.
    if (in.fail() || in.eof()) {
        return nullopt; 
    }

    vector<pair<int, int>> pairs;

    for (int i = 0; i < MAX_PAIRS; ++i) {
        // Process the current pair we are looking at in the file.
        pair<int, int> current_pair;
        in.read(reinterpret_cast<char*>(&current_pair.first), sizeof(int));
        
        // Stop reading if we hit the end of the file.
        if (in.eof()) {
            break;
        }
        
        // Read value.
        in.read(reinterpret_cast<char*>(&current_pair.second), sizeof(int));

        if (in.eof()) {
            break;
        }

        pairs.push_back(current_pair);
    }

    in.close();
    return pairs; // Return the read key-value pairs.
}