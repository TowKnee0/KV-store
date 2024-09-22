#include "io.h"
#include<fstream>
#include <vector>
#include <utility>
using namespace std;

/**
 * When we write data to a file we will fill it in 4KB pages.
 * Note that key and value are 4-byte integers each, so a key value pair will take up 8-bytes.
 * This suggests each 4KB or 4096byte page can hold up to 4096 / 8 = 512 key value pairs.
 * If we use plain text we will use up extra space for the delimiters. And the performance is worse anyway. 
 * We will write the information in a binary format to be more performant.
 */
const int PAGE_SIZE = 4096; // 4KB page size
const int PAIR_SIZE = sizeof(int) * 2; // Size of one key-value pair (2 integers)
const int MAX_PAIRS = PAGE_SIZE / PAIR_SIZE;

int writeToFile(const string& filename, const vector<pair<int, int>>& data) {
    ofstream out(filename, ios_base::binary);
    
    // Handle error opening file
    if (!out) {
        remove(filename.c_str());
        return -1;
    }


    // Try to write all key-value pairs
    bool success = true;
    for (const pair<int, int>& current_pair : data) {
        if (!out.write(reinterpret_cast<const char*>(&current_pair.first), sizeof(int))) {
            success = false;
            break; 
        }
        if (!out.write(reinterpret_cast<const char*>(&current_pair.second), sizeof(int))) {
            success = false;
            break;
        }
    }

    out.close(); // Close the file

    // If any write failed, clean up.
    if (!success) {
        remove(filename.c_str());
        return -1;
    }

    return 1;
}

optional<vector<pair<int, int>>> readPageFromFile(const string filename, int pageNumber) {
    ifstream in(filename, ios_base::binary);
    
    // pageNumber is 1-indexed
    size_t offset = (pageNumber - 1) * PAGE_SIZE;

    in.seekg(offset);
    
     // Check if the seek was successful or if we are at the end of the file
    if (in.fail() || in.eof()) {
        return nullopt; 
    }

    vector<pair<int, int>> pairs;

    for (int i = 0; i < MAX_PAIRS; ++i) {
        // Process the current pair we are looking at in the file.
        pair<int, int> current_pair;
        in.read(reinterpret_cast<char*>(&current_pair.first), sizeof(int));
        
        // Stop reading if we hit the end of the file
        if (in.eof()) {
            break;
        }
        
        // Read value
        in.read(reinterpret_cast<char*>(&current_pair.second), sizeof(int));

        if (in.eof()) {
            break;
        }

        pairs.push_back(current_pair);
    }

    in.close();
    return pairs; // Return the read key-value pairs
}