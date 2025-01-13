#include "btree.h"
#include <iostream>
#include <sstream>
#include <QString>
#include <QTextStream>

BTreeNode::BTreeNode(bool leaf) {
    this->leaf = leaf;
    numKeys = 0;
}

void BTreeNode::traverse(std::ostream& out) {
    for (int i = 0; i < numKeys; i++) {
        if (!leaf) children[i]->traverse(out);
        out << keys[i] << " " << data[i] << "\n";
    }
    if (!leaf) {
        children[numKeys]->traverse(out);
    }
}

void BTreeNode::traverse(std::ostream& stream, const std::string prefix) {
    stream << prefix << " -> " << (void*)(this) << "  {";
    stream << " keys: [";
    stream << keys[0];
    for (int i = 1; i < numKeys; i++) {
        stream << ", " << keys[i];
    }
    stream << "]; data = [";

    stream << data[0];
    for (int i = 1; i < numKeys; i++) {
        stream << ", " << data[i];
    }
    stream << "] }\n";
    if (leaf) {
        return;
    }
    for (int i = 0; i < numKeys + 1; i++) {
        printNode(stream, "     " + prefix, children[i]);
    }
}
void printNode(std::ostream& stream, const std::string prefix, const BTreeNode* node)
{
    stream << prefix << " -> " << (void*)node << "  {";
    stream << " keys: [";
    stream << node->keys[0];
    for (int i = 1; i < node->numKeys; i++) {
        stream << ", " << node->keys[i];
    }
    stream << "]; data = [";

    stream << node->data[0];
    for (int i = 1; i < node->numKeys; i++) {
        stream << ", " << node->data[i];
    }
    stream << "] }\n";
    if (node->leaf) {
        return;
    }
    for (int i = 0; i < node->numKeys + 1; i++) {
        printNode(stream, "     " + prefix, node->children[i]);
    }
}

void BTreeNode::insertNonFull(int key, const std::string& dataVal) {
    int i = numKeys - 1;
    if (leaf) {
        while (i >= 0 && keys[i] > key) {
            keys[i + 1] = keys[i];
            data[i + 1] = data[i];
            i--;
        }
        keys[i + 1] = key;
        data[i + 1] = dataVal;
        numKeys++;
    } else {
        while (i >= 0 && keys[i] > key) i--;
        i++;
        if (children[i] && children[i]->numKeys == MAX_KEY_T) {
            splitChild(i, children[i]);
            if (keys[i] < key) {
                i++;
            }
        }
        children[i]->insertNonFull(key, dataVal);

    }
}

void BTreeNode::splitChild(int i, BTreeNode* y) {
    BTreeNode* z = new BTreeNode(y->leaf);
    z->numKeys = T1000 - 1;
    for (int j = 0; j < T1000 - 1; j++) {
        z->keys[j] = y->keys[j + T1000];
    }
    for (int j = 0; j < T1000 - 1; j++) {
        z->data[j] = y->data[j + T1000];
    }
    if (!y->leaf) {
        for (int j = 0; j < T1000; j++) {
            z->children[j] = y->children[j + T1000];
        }
    }
    y->numKeys = T1000 - 1;
    for (int j = numKeys; j >= i + 1; j--) {
        children[j + 1] = children[j];
    }
    children[i + 1] = z;
    for (int j = numKeys - 1; j >= i; j--) {
        keys[j + 1] = keys[j];
        data[j + 1] = data[j];
    }
    keys[i] = y->keys[T1000 - 1];
    data[i] = y->data[T1000 - 1];
    numKeys++;
}

/*std::optional<std::pair<std::string, BTreeNode*>> BTreeNode::search(int key, int& comparisons) {
    int left = 0;
    int right = numKeys - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        comparisons++;
        if (keys[mid] == key) {
            return std::optional(std::make_pair(data[mid], this));
        } else if (keys[mid] < key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    if (leaf) {
        return std::nullopt;
    }
    return children[left]->search(key, comparisons);
}*/

std::optional<std::pair<std::string, BTreeNode*>> BTreeNode::search(int key, int& comparisons) {

    if (!leaf) {
        if (key < keys[0]) {
            comparisons++;
            return children[0]->search(key, comparisons);
        } else if (key > keys[numKeys-1]) {
            comparisons++;
            return children[numKeys]->search(key, comparisons);
        }
    }

    int i = numKeys / 2 + 1;
    int delta = numKeys / 2;

    while (delta > 0) {
        comparisons++;
        if (i <= numKeys && i > 0 && key == keys[i-1]) {
            return std::optional(std::make_pair(data[i-1], this));
        }
        comparisons++;
        if (i > numKeys || (i > 0 && key < keys[i-1])) {
            delta /= 2;
            i -= delta + 1;
        } else {
            delta /= 2;
            i += delta + 1;
        }
    }

    comparisons++;
    if (i > 0 && key == keys[i-1]) {
        return std::optional(std::make_pair(data[i-1], this));
    }
    comparisons++;
    if (leaf) {
        return std::nullopt;
    }

    comparisons++;
    i += (key > keys[i-1]) ? 1 : 0;
    return children[i-1]->search(key, comparisons);
}

bool BTreeNode::remove(BTreeNode** node_ptr, int key) {
    BTreeNode* node = *node_ptr;
    int idx = 0;
    while (idx < node->numKeys && node->keys[idx] < key) {
        idx++;
    }
    if (idx < node->numKeys && node->keys[idx] == key) {
        if (node->leaf) {
            for (int i = idx; i < node->numKeys - 1; i++) {
                node->keys[i] = node->keys[i + 1];
                node->data[i] = node->data[i + 1];
            }
            node->numKeys--;
            return true;
        } else {
            if (node->children[idx]->numKeys >= T1000) {
                auto pred = node->getPredecessor(idx);
                node->keys[idx] = pred.first;
                node->data[idx] = pred.second;
                return BTreeNode::remove(&node->children[idx], pred.first);
            } else if (node->children[idx + 1]->numKeys >= T1000) {
                auto succ = node->getSuccessor(idx);
                node->keys[idx] = succ.first;
                node->data[idx] = succ.second;
                return BTreeNode::remove(&node->children[idx + 1], succ.first);
            } else {
                merge_child_keys(&node, idx);
                return BTreeNode::remove(&node->children[idx], key);
            }
        }
    } else {
        if (node->leaf) {
            return false;
        }
        if (node->children[idx]->numKeys < T1000) {
            fill(&node, idx);
        }
        node->traverse(std::cout, "");
        std::cout << "\n" << std::endl;
        return BTreeNode::remove(&node->children[idx], key);
    }
}

std::pair<int, std::string> BTreeNode::getPredecessor(int idx) {
    BTreeNode* cur = children[idx];
    while (!cur->leaf) {
        cur = cur->children[cur->numKeys];
    }
    return std::make_pair(cur->keys[cur->numKeys - 1], cur->data[cur->numKeys - 1]);
}

std::pair<int, std::string> BTreeNode::getSuccessor(int idx) {
    BTreeNode* cur = children[idx + 1];
    while (!cur->leaf) {
        cur = cur->children[0];
    }
    return std::make_pair(cur->keys[0], cur->data[0]);
}

void BTreeNode::fill(BTreeNode** node_ptr, int idx) {
    BTreeNode* node = *node_ptr;
    if (idx != 0 && node->children[idx - 1]->numKeys >= T1000) {
        borrow_from_prev(node_ptr, idx);
    } else if (idx != node->numKeys && node->children[idx + 1]->numKeys >= T1000) {
        borrow_from_next(node_ptr, idx);
    } else {
        if (idx != node->numKeys) {
            merge_child_keys(node_ptr, idx);
        } else {
            merge_child_keys(node_ptr, idx - 1);
        }
    }
}

void BTreeNode::borrow_from_prev(BTreeNode** node_ptr, int idx) {
    BTreeNode* node = *node_ptr;
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx - 1];
    for (int i = child->numKeys - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
        child->data[i + 1] = child->data[i];
    }
    if (!child->leaf) {
        for (int i = child->numKeys; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }
    child->keys[0] = node->keys[idx - 1];
    if (!child->leaf) {
        child->children[0] = sibling->children[sibling->numKeys];
    }
    node->keys[idx - 1] = sibling->keys[sibling->numKeys - 1];
    node->data[idx - 1] = sibling->data[sibling->numKeys - 1];
    child->numKeys++;
    sibling->numKeys--;
}

void BTreeNode::borrow_from_next(BTreeNode** node_ptr, int idx) {
    BTreeNode* node = *node_ptr;
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];
    child->keys[child->numKeys] = node->keys[idx];
    child->data[child->numKeys] = node->data[idx];
    if (!child->leaf) {
        child->children[child->numKeys + 1] = sibling->children[0];
    }
    node->keys[idx] = sibling->keys[0];
    node->data[idx] = sibling->data[0];
    for (int i = 1; i < sibling->numKeys; i++) {
        sibling->keys[i - 1] = sibling->keys[i];
        sibling->data[i - 1] = sibling->data[i];
    }
    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->numKeys; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    child->numKeys++;
    sibling->numKeys--;
}

void BTreeNode::merge_child_keys(BTreeNode** node_ptr, int idx)
{
    BTreeNode* node = *node_ptr;

    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];
    child->keys[T1000 - 1] = node->keys[idx];
    child->data[T1000 - 1] = node->data[idx];
    for (int i = 0; i < sibling->numKeys; i++) {
        child->keys[i + T1000] = sibling->keys[i];
        child->data[i + T1000] = sibling->data[i];
    }
    if (!child->leaf) {
        for (int i = 0; i <= sibling->numKeys; i++) {
            child->children[i + T1000] = sibling->children[i];
        }
    }
    for (int i = idx + 1; i < node->numKeys; i++) {
        node->keys[i - 1] = node->keys[i];
        node->data[i - 1] = node->data[i];
        node->children[i] = node->children[i + 1];
    }
    child->numKeys += sibling->numKeys + 1;
    node->numKeys--;
    std::cout << "--- Merge --- " << idx << "\n";
    node->traverse(std::cout, "");
    for (int i = 0; i < sibling->children.size(); i++) {
        sibling->children[i] = nullptr;
    }
    delete sibling;
    std::cout << "--- After change root --- " << idx << "\n";
    (*node_ptr)->traverse(std::cout, "");
    std::cout << "------------------------- " << idx << "\n";
}

BTree::BTree() {
    root = nullptr;
}

bool BTree::insert(int key, const std::string& dataVal) {
    int comparisons = 0;
    if (search(key, comparisons).has_value()) {
        return false;
    }
    if (!root) {
        root = new BTreeNode(true);
        root->keys[0] = key;
        root->data[0] = dataVal;
        root->numKeys = 1;
    } else {
        if (root->numKeys == MAX_KEY_T) {
            BTreeNode* s = new BTreeNode(false);
            s->children[0] = root;
            s->splitChild(0, root);
            int i = (s->keys[0] < key) ? 1 : 0;
            s->children[i]->insertNonFull(key, dataVal);
            root = s;
        } else {
            root->insertNonFull(key, dataVal);
        }
    }
    return true;
}

std::optional<std::pair<std::string, BTreeNode*>> BTree::search(int key, int& comparisons) {
    comparisons = 0;
    return root ? root->search(key, comparisons) : std::nullopt;
}

bool BTree::edit(int key, const std::string& dataVal) {
    std::string tempData;
    int comparisons = 0;
    std::optional<std::pair<std::string, BTreeNode*>> res = search(key, comparisons);
    if (res.has_value()) {
        BTreeNode* node = res.value().second;
        for (int i = 0; i < node->numKeys; i++) {
            if (node->keys[i] == key) {
                node->data[i] = dataVal;
                break;
            }
        }
        return true;
    }
    return false;
}

bool BTree::remove(int key) {
    if (!root) {
        return false;
    }
    bool result = BTreeNode::remove(&root, key);
    if (root->numKeys == 0) {
        BTreeNode* temp = root;
        root = root->leaf ? nullptr : root->children[0];
        for (int i = 0; i < temp->children.size(); i++) {
            temp->children[i] = nullptr;
        }
        delete temp;
    }
    return result;
}

void BTree::save(std::ostream& out) {
    if (root) {
        root->traverse(out);
    }
}

void BTree::load(std::istream& in) {
    delete root;
    root = nullptr;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line, ' ');
        if (line.empty()) {
            continue;
        }
        int key = std::stoi(line);
        std::string data;
        std::getline(in, data);
        insert(key, data);
    }
}

void BTree::save(QTextStream &out) {
    std::ostringstream os;
    save(os);
    QString data = QString::fromStdString(os.str());
    out << data;
}

void BTree::load(QTextStream &in) {
    QString data = in.readAll();
    std::istringstream is(data.toStdString());
    load(is);
}


BTreeNode::~BTreeNode() {
    if (!leaf) {
        for (int i = 0; i <= numKeys; i++) {
            delete children[i];
        }
    }
}

BTree::~BTree() {
    if (root) delete root;
}

std::ostream& operator<<(std::ostream& stream, const BTree& tree)
{
    if (tree.root == nullptr) {
        stream << "Empty tree\n";
    }
    tree.root->traverse(stream, "");
    stream << std::endl;
    return stream;
}
