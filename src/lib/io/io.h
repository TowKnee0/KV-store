/**
 * If we do I/O in multiple different places we anticipate that we will run into pointer and misalign issues.
 * So we will handle all Direct I/O from a centralized location.
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

class IOHandler {
    public:
        IOHandler(const string dbname);
        
        /*
        * Write key-value pairs to a binary file.
        * Returns 1 if successful and -1 if unsuccessful.
        * Parameters:
        *  - filename: the name of the file to write to.
        *  - data: vector of key-value pairs to write into the file named filename.
        */
        int writeToFile(const string filename, const vector<pair<int, int>> &data);

        /*
        * Read a specific page (4KB) of key-value pairs from a binary file.
        * Returns an vector of the found key-value pairs within the page if successful, or {} if reading fails.
        *
        * Parameters:
        *  - filename: the name of the file to read from.
        *  - pageNumber: the page number to read from. Page numbers are 1-indexed.
        */
        optional<vector<pair<int, int>>> readPageFromFile(const string filename, int pageNumber);

        /*
        * Get the number of pages in a file.
        * Returns the number of pages in the file if successful, or -1 if reading fails.
        *
        * Parameters:
        *  - filename: the name of the file to read from.
        */
        int numPagesInFile(const string filename);

        /*
        * Get all sst filenames associated with a specific db instance.
        * Returns a vector of filenames if successful, or {} which is the empty vector if no files are found.
        * Should be sorted (ie oldest first, newest last).
        *
        */
        vector<string> getDbFilenames();

    private:
        string dbname;
};

#endif