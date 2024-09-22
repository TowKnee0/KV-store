#include "sst.h"
#include "../lib/io/io.h"
#include <iostream>


using namespace std;

SST::SST(string filename, string dbname) {
    filename = filename;
    dbname = dbname;
}

int SST::write(const vector<pair<int, int>>& data) {
    if (writeToFile(filename, data) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

optional<int> SST::get(int key){}

vector<entry>* SST::scan(int rangeStart, int rangeEnd){}