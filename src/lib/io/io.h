/**
 * If we do I/O in multiple different places we anticipate that we will run into pointer and misalign issues.
 * So we will handle all i/o from a centralized location.
 * What we want to support:
 * - Writing to a file
 * - Reading a specific page in a file starting from some offset
 */

#ifndef IO_H
#define IO_H

#include <string>
#include <vector>
#include <optional>

using namespace std; 

enum ErrorCode {
    WRITE_FAILURE
};

/*
 * Write key-value pairs to a binary file.
 * Parameters:
 *  - filename: the name of the file to write to.
 *  - data: vector of key-value pairs to write into the file named filename.
 */
ErrorCode writeToFile(const string filename, const vector<pair<int, int>>& data);

/*
 * Read a specific page (4KB) of key-value pairs from a binary file.
 * Returns an vector of the found key-value pairs within the page if successful, or {} if reading fails.
 * 
 * Parameters:
 *  - filename: the name of the file to read from.
 *  - pageNumber: the page number to read from.
 */
optional<vector<pair<int, int>>> readPageFromFile(const string filename, int pageNumber);

#endif