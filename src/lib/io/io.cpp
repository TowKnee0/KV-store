#include "io.h"
#include<fstream>

using namespace std;

/**
 * When we write data to a file we will fill it in 4KB pages.
 * Note that key and value are 4-byte integers each, so a key value pair will take up 8-bytes.
 * This suggests each 4KB or 4096byte page can hold up to 4096 / 8 = 512 key value pairs.
 * We need to write in a binary format so that it is convenient to hop in the binary search using pread I guess.
 * Because if we use plain text we will use up extra space for the delimiters. 
 */
const int PAGE_SIZE = 4096; // 4KB page size
const int PAIR_SIZE = sizeof(int) * 2; // Size of one key-value pair (2 integers)
const int MAX_PAIRS = PAGE_SIZE / PAIR_SIZE;

/**
 * https://linux.die.net/man/2/open
 */
int writeToFile(const string filename, const vector<pair<int, int>>& data) {
    ;
}