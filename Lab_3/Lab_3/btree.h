#ifndef BTREE_H
#define BTREE_H

#include <string>
#include <array>
#include <vector>
#include <optional>
#include <QTextStream>

constexpr int T1000 = 10;
constexpr int MAX_KEY_T = 2 * T1000 - 1;

class BTree;
class BTreeNode;
std::ostream& operator<<(std::ostream& stream, const BTree& tree);
//void printNode(std::ostream& stream, const std::string prefix, const BTreeNode* node);

struct Tree_tag {
    BTreeNode* node;
    int keysCount;
    int idx;
};

class BTreeNode {
    friend void printNode(std::ostream& stream, const std::string prefix, const BTreeNode* node);
public:
    std::array<int, MAX_KEY_T> keys;
    std::array<std::string, MAX_KEY_T> data;
    std::array<BTreeNode*, MAX_KEY_T + 1> children;
    int numKeys;
    bool leaf;

    BTreeNode(bool leaf);
    ~BTreeNode();

    void traverse(std::ostream& out);
    void traverse(std::ostream& stream, const std::string prefix);
    void insertNonFull(int key, const std::string &dataVal);
    void splitChild(int i, BTreeNode *y);
    std::optional<std::pair<std::string, BTreeNode*>> search(int key, int& comparisons);
    static bool remove(BTreeNode** node_ptr, int key);
    std::pair<int, std::string> getPredecessor(int idx);
    std::pair<int, std::string> getSuccessor(int idx);
    static void fill(BTreeNode** node_ptr, int idx);
    static void borrow_from_prev(BTreeNode** node_ptr, int idx);
    static void borrow_from_next(BTreeNode** node_ptr, int idx);
    static void merge_child_keys(BTreeNode** node_ptr, int idx);
};

class BTree {
    friend std::ostream& operator<<(std::ostream& stream, const BTree& tree);
public:
    BTree();
    bool insert(int key, const std::string &dataVal);
    std::optional<std::pair<std::string, BTreeNode*>> search(int key, int& comparisons);
    bool edit(int key, const std::string &dataVal);
    bool remove(int key);
    void save(QTextStream &out);
    void load(QTextStream &in);
    void save(std::ostream& out);
    void load(std::istream& in);
    ~BTree();


private:
    BTreeNode *root;
};

#endif // BTREE_H
