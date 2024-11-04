/**
 * This is for structuring an SST as a Static B-Tree.
 * The goal is to have 1 B-Tree for each SST.
 * 
 * From handout:
 * - The B-Tree is Static in the sense that it should not handle insertions, updates, or deletes.
 * - The existing sorted data can be thought of as contiguously allocated 4kb leaf nodes with no padding
 *   (i.e., there is no spare space in each leaf node to accommodate insertions). 
 * 
 * For both get and scan, the binary search over leaves option will search only from the leaves and never go to root or internal node of the B-Tree
 *
 * For the other option:
 * - get() will start from root and go to the corresponding leaf and do a binary search on that leaf, [To implement after building internal node logic is done]
 * - scan() will first get() the leaf that may have the lower key walk the B tree until the upper key. [To implement after building internal node logic is done]
 *
 */
#include <vector>

using namespace std;

class BTree {
private:
    int PAGE_SIZE = 4096;   // Page size in bytes
    int ENTRY_SIZE = 8;     // Size of each entry (key-pointer pair)
    int MAX_ENTRIES = PAGE_SIZE / ENTRY_SIZE;  // Max entries per page
public:
    /**
     * This is going to builds the internal nodes of a B-tree layer-by-layer. It does this by starting from the initial provided leaf pages and building upwards.
     * 
     * On any given layer, the internal nodes are represented by a Vector of key-pointer pairs. A pointer links to a child page on the lower layer.
     * If the internal node is on the bottom-most layer, then the pointer is pointing to a leaf node in which case we would need to do direct I/O.
     *
     * Parameters:
     *  - pages: Leaf nodes, where each vector<pair<int, int>> represents a page of sorted key-value pairs.
     * 
     * Returns:
     *  - A vector of vectors, where each inner vector represents an internal node's key-pointer pairs.
     */
    vector<vector<pair<int, int>>> createInternalNodes(vector<vector<pair<int, int>>> pages);
};
