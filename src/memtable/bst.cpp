#include "bst.h"
#include <optional>
#include <iostream>
#include <memory>

using namespace std;

// implementations were taken directly from CLRS pseudocode.
// only addition made is the use for smart pointers.
Bst::Bst(int memtable_size) : max_size(memtable_size), c_size(0), root(nullptr) {}

void Bst::leftRotate(shared_ptr<node> x) {
    if (!x || !x->right)
        return;

    auto y = x->right;
    x->right = y->left;

    if (x->right) {
        x->right->parent = x;
    }
    y->parent = x->parent;
    if (auto parentPtr = x->parent.lock()) {
        if (x == parentPtr->left) {
            parentPtr->left = y;
        } else {
            parentPtr->right = y;
        }
    } else {
        root = y;
    }

    y->left = x;
    x->parent = y;
}

void Bst::rightRotate(shared_ptr<node> y) {
    if (!y || !y->left)
        return;

    auto x = y->left;
    y->left = x->right;

    if (y->left) {
        y->left->parent = y;
    }
    x->parent = y->parent;
    if (auto parentPtr = y->parent.lock()) {
        if (y == parentPtr->right) {
            parentPtr->right = x;
        } else {
            parentPtr->left = x;
        }
    } else {
        root = x;
    }

    x->right = y;
    y->parent = x;
}

ErrorCode Bst::put(int key, int value) {
    if (c_size == max_size) {
        return TREE_FULL;
    }

    shared_ptr<node> x = root;
    shared_ptr<node> y = nullptr;
    auto z = make_shared<node>();
    z->key = key;
    z->value = value;

    while (x != nullptr) {
        y = x;
        if (z->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;
    if (!y) {
        root = z;
    } else if (z->key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }
    z->left = nullptr;
    z->right = nullptr;
    z->color = RED;
    fix(z);
    c_size++;
    return SUCCESS;
}

void Bst::fix(shared_ptr<node> z) {
    while (z != root && z->parent.lock()->color == RED) {
        if (z->parent.lock() == z->parent.lock()->parent.lock()->left) {
            auto y = z->parent.lock()->parent.lock()->right;
            if (y && y->color == RED) {
                z->parent.lock()->color = BLACK;
                y->color = BLACK;
                z->parent.lock()->parent.lock()->color = RED;
                z = z->parent.lock()->parent.lock();
            } else {
                if (z == z->parent.lock()->right) {
                    z = z->parent.lock();
                    leftRotate(z);
                }
                z->parent.lock()->color = BLACK;
                z->parent.lock()->parent.lock()->color = RED;
                rightRotate(z->parent.lock()->parent.lock());
            }
        } else {
            auto y = z->parent.lock()->parent.lock()->left;
            if (y && y->color == RED) {
                z->parent.lock()->color = BLACK;
                y->color = BLACK;
                z->parent.lock()->parent.lock()->color = RED;
                z = z->parent.lock()->parent.lock();
            } else {
                if (z == z->parent.lock()->left) {
                    z = z->parent.lock();
                    rightRotate(z);
                }
                z->parent.lock()->color = BLACK;
                z->parent.lock()->parent.lock()->color = RED;
                leftRotate(z->parent.lock()->parent.lock());
            }
        }
    }
    root->color = BLACK;
}

optional<int> Bst::get(int key) {
    shared_ptr<node> curr = root;
    while (curr != nullptr) {
        if (key == curr->key) {
            return curr->value;
        } else if (key < curr->key) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    return nullopt;
}
