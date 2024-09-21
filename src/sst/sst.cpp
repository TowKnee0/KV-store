#include "sst.h"
#include "../lib/io/io.h"
#include <iostream>


using namespace std;

SST::SST(const vector<pair<int, int>>& data, string filename) {
    filename = filename;
    if (writeToFile(filename, data) == -1) {
        throw ios_base::failure("Failed to write to file " + filename);
    }
}