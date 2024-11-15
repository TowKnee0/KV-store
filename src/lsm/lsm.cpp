#include "lsm.h"


Lsm::Lsm(shared_ptr<Memtable> buffer) {
    this->buffer = buffer;
}

void Lsm::put(int key, int value) {
    if (this->buffer->put(key, value) == EXIT_SUCCESS) {
        return;
    }

    vector<pair<int, int>> data = this->buffer->inOrder();
    
}