#include "lsm.h"
#include <algorithm>
#include <stdexcept>

Lsm::Lsm(string dbname, shared_ptr<Buffer> buffer, shared_ptr<Memtable> memtable, shared_ptr<IO> io) {
    this->buffer = buffer;
    this->dbname = dbname;
    this->io = io;
    this->memtable = memtable;
}

void Lsm::put(int key, int value) {
    if (this->memtable->put(key, value) == EXIT_SUCCESS) {
        return;
    }

    vector<pair<int, int>> data = this->memtable->inOrder();
    this->memtable->clear();

    BTree temp = BTree("temp", this->dbname, io, this->buffer);
    temp.write(data);

    for (int i = 0; i < this->trees.size(); i++) {
        shared_ptr<BTree> tree = this->trees[i];
        if (tree->filename.find(this->level_prefix) != 0) {
            throw runtime_error("Invalid BTree name: '" + tree->filename +"' does not contain level_prefix");
        }

        int clevel = i + 1;

        // empty level
        if (tree == nullptr) {
            if (this->trees[clevel - 1] != nullptr) {
                throw runtime_error("Attempt to overwrite existing BTree at index: " + to_string(clevel - 1));
            }

            temp.rename(this->level_prefix + to_string(clevel));
            this->trees[clevel-1] = make_shared<BTree>(temp);
            return;
        }

        int tlevel = stoi(tree->filename.substr(this->level_prefix.length()));

        BTree merged = BTree("merged", this->dbname, io, this->buffer);
        merged.mergeBTrees(tree->filename, temp.filename);
        
        io->del(temp.filename);
        io->del(tree->filename);

        merged.rename("temp");
        temp = merged;
    }

    temp.rename(this->level_prefix + to_string(this->trees.size()+1));
    this->trees.push_back(make_shared<BTree>(temp));
}

void Lsm::del(int key) {
    this->put(key, this->sentinel);
}

optional<int> Lsm::get(int key) {
    if (int ret = this->memtable->get(key) != nullopt) {
        if (ret == INT_MIN) {
            return nullopt;
        }
        return ret;
    }

    for (shared_ptr<BTree> tree: this->trees) {
        if (tree == nullptr) {
            continue;
        }

        if (int ret = tree->get(key) != nullopt) {
            if (ret == INT_MIN) {
                return nullopt;
            }
            return ret;
        }
    }
    return nullopt;
}


void Lsm::load() {
    int mpages = io->numPagesInFile("memtable");
    if (mpages != 0) {
        for (int p  = 0; p < mpages; p++) {
            optional<vector<pair<int, int>>> data = this->io->readPageFromFile("memtable", p+1);
            if (!data.has_value()) {
                continue;
            }

            for (pair<int, int> pear : data.value()) {
                this->memtable->put(pear.first, pear.second);
            }
        }
    }
    io->del("memtable");

    vector<string> files = this->io->getDbFilenames();
    for (string file: files) {
        string f = file.substr(0, file.find('.'));
        if (f.find(this->level_prefix) != 0) {
            continue;
        }

        int tlevel = stoi(f.substr(this->level_prefix.length()));
        if (this->trees.size() < tlevel) {
            this->trees.resize(tlevel);
        }
        this->trees[tlevel - 1] = make_shared<BTree>(f, this->dbname, this->io, this->buffer); 
    }
}

void Lsm::save() {
    vector<pair<int, int>> data = this->memtable->inOrder();
    this->io->writeToFile("memtable", data);
}