/**
 * This is for structuring an SST as a Static B-Tree.
 * The goal is to have 1 B-Tree for each SST.
 * 
 * From handout:
 * - The B-Tree is Static in the sense that it should not handle insertions, updates, or deletes.
 * - The existing sorted data can be thought of as contiguously allocated 4kb leaf nodes with no padding
 *   (i.e., there is no spare space in each leaf node to accommodate insertions). 
 * 
 * -
 */
#ifndef BTREE_H
#define BTREE_H

#include <vector>

using namespace std;

// Constants for B-Tree degree
const int BTREE_DEGREE = 4;

// Structure representing a B-Tree node.
struct BTreeNode {
    bool isLeaf;                     // True if the node is a leaf
    vector<pair<int, int>> entries; 
    vector<BTreeNode*> children;     // Child pointers (empty if it's a leaf)

    // Constructor to initialize a B-Tree node as a leaf or as an internal node
    BTreeNode(bool leaf) : isLeaf(leaf) {}
};

class BTree {
    private:
        BTreeNode* root;           
        int degree;                      

    public:
        BTree(int degree = BTREE_DEGREE);

        // Build B-Tree using the provided data)
        void buildBTree(const vector<pair<int, int>>& data);
        
        // Retrieve the value for a given key
        int get(int key);

        // Would need scan too
    };

#endif