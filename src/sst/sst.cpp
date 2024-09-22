#include "sst.h"
#include "../lib/io/io.h"
#include <iostream>


using namespace std;

Sst::Sst(string filename, string dbname) {
    filename = filename;
    dbname = dbname;
}

int Sst::write(const vector<pair<int, int>>& data) {
    if (writeToFile(filename, data) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

optional<int> Sst::get(int key){}

vector<entry>* Sst::scan(int rangeStart, int rangeEnd){}