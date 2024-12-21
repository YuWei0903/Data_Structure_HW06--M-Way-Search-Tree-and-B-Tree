#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
#include <algorithm>

using namespace std;

// 節點結構 (適用於 m-way 搜尋樹和 B-tree)
struct Node {
    vector<int> keys;           // 儲存關鍵值
    vector<Node*> children;     // 子節點指標

    Node() {}

    // 檢查是否為葉子節點
    bool isLeaf() {
        return children.empty();
    }
};

// m-way 搜尋樹類別
class MWayTree {
private:
    int m;              // 樹的階數
    Node* root;         // 樹的根節點

    // 插入輔助函式 (遞歸)
    void insertNonFull(Node* node, int key) {
        if (node->isLeaf()) {
            // 將關鍵值插入葉子節點
            node->keys.push_back(key);
            sort(node->keys.begin(), node->keys.end());
        }
        else {
            // 找到適當的子節點
            size_t i = 0;
            while (i < node->keys.size() && key > node->keys[i]) {
                i++;
            }
            Node* child = node->children[i];
            if (child->keys.size() == m - 1) {
                // 如果子節點滿了，先進行分裂
                splitChild(node, i);
                if (key > node->keys[i]) {
                    child = node->children[i + 1];
                }
            }
            insertNonFull(child, key);
        }
    }

    // 分裂子節點
    void splitChild(Node* parent, int index) {
        Node* fullChild = parent->children[index];
        Node* newChild = new Node();

        // 將 fullChild 的中位數升級到父節點
        int median = fullChild->keys[m / 2];
        parent->keys.insert(parent->keys.begin() + index, median);
        parent->children.insert(parent->children.begin() + index + 1, newChild);

        // 分割 fullChild
        newChild->keys.assign(fullChild->keys.begin() + m / 2 + 1, fullChild->keys.end());
        fullChild->keys.resize(m / 2);

        if (!fullChild->isLeaf()) {
            newChild->children.assign(fullChild->children.begin() + m / 2 + 1, fullChild->children.end());
            fullChild->children.resize(m / 2 + 1);
        }
    }

    // 刪除輔助函式
    bool deleteFromNode(Node* node, int key) {
        int idx = findKey(node, key);
        if (idx != -1) {
            // 找到該關鍵值並刪除
            if (node->isLeaf()) {
                // 直接從葉子節點中刪除
                node->keys.erase(node->keys.begin() + idx);
                return true;
            }
            else {
                // 若是內部節點，則需要處理
                return deleteInternalNode(node, idx);
            }
        }
        else {
            // 如果該關鍵值不在當前節點，則遞歸刪除
            return deleteFromChild(node, key);
        }
    }

    // 查找關鍵值在節點中的索引
    int findKey(Node* node, int key) {
        for (size_t i = 0; i < node->keys.size(); i++) {
            if (node->keys[i] == key) return i;
        }
        return -1;
    }

    // 刪除內部節點的關鍵值
    bool deleteInternalNode(Node* node, int idx) {
        Node* leftChild = node->children[idx];
        Node* rightChild = node->children[idx + 1];

        if (leftChild->keys.size() >= m / 2) {
            // 從左子樹取得最大鍵值
            int predecessor = leftChild->keys.back();
            deleteFromNode(leftChild, predecessor);
            node->keys[idx] = predecessor;
        }
        else if (rightChild->keys.size() >= m / 2) {
            // 從右子樹取得最小鍵值
            int successor = rightChild->keys.front();
            deleteFromNode(rightChild, successor);
            node->keys[idx] = successor;
        }
        else {
            // 合併左右子樹
            mergeChildren(node, idx);
            deleteFromNode(leftChild, node->keys[idx]);
        }
        return true;
    }

    // 合併兩個子樹
    void mergeChildren(Node* parent, int idx) {
        Node* leftChild = parent->children[idx];
        Node* rightChild = parent->children[idx + 1];

        // 合併父節點的鍵值與右子樹
        leftChild->keys.push_back(parent->keys[idx]);
        leftChild->keys.insert(leftChild->keys.end(), rightChild->keys.begin(), rightChild->keys.end());

        if (!leftChild->isLeaf()) {
            leftChild->children.insert(leftChild->children.end(), rightChild->children.begin(), rightChild->children.end());
        }

        // 移除父節點中的鍵值和右子樹
        parent->keys.erase(parent->keys.begin() + idx);
        parent->children.erase(parent->children.begin() + idx + 1);
        delete rightChild;
    }

    // 刪除子節點中的關鍵值
    bool deleteFromChild(Node* parent, int key) {
        int idx = 0;
        while (idx < parent->keys.size() && key > parent->keys[idx]) {
            idx++;
        }
        Node* child = parent->children[idx];
        if (child->keys.size() == m / 2 - 1) {
            // 如果子節點少於 m / 2，則需要重組
            rebalanceChild(parent, idx);
        }
        return deleteFromNode(parent->children[idx], key);
    }

    // 重組子節點
    void rebalanceChild(Node* parent, int idx) {
        Node* child = parent->children[idx];
        if (idx > 0 && parent->children[idx - 1]->keys.size() > m / 2 - 1) {
            // 從左子節點借鍵值
            borrowFromLeft(parent, idx);
        }
        else if (idx < parent->keys.size() && parent->children[idx + 1]->keys.size() > m / 2 - 1) {
            // 從右子節點借鍵值
            borrowFromRight(parent, idx);
        }
        else {
            // 合併子節點
            if (idx > 0) {
                mergeChildren(parent, idx - 1);
            }
            else {
                mergeChildren(parent, idx);
            }
        }
    }

    // 從左子節點借鍵值
    void borrowFromLeft(Node* parent, int idx) {
        Node* child = parent->children[idx];
        Node* leftSibling = parent->children[idx - 1];

        child->keys.insert(child->keys.begin(), parent->keys[idx - 1]);
        parent->keys[idx - 1] = leftSibling->keys.back();
        leftSibling->keys.pop_back();

        if (!leftSibling->isLeaf()) {
            child->children.insert(child->children.begin(), leftSibling->children.back());
            leftSibling->children.pop_back();
        }
    }

    // 從右子節點借鍵值
    void borrowFromRight(Node* parent, int idx) {
        Node* child = parent->children[idx];
        Node* rightSibling = parent->children[idx + 1];

        child->keys.push_back(parent->keys[idx]);
        parent->keys[idx] = rightSibling->keys.front();
        rightSibling->keys.erase(rightSibling->keys.begin());

        if (!rightSibling->isLeaf()) {
            child->children.push_back(rightSibling->children.front());
            rightSibling->children.erase(rightSibling->children.begin());
        }
    }

public:
    MWayTree(int m) : m(m), root(nullptr) {}

    Node* getRoot() { return root; }

    // 插入功能 (簡化版)
    void insert(int key) {
        if (!root) {
            root = new Node();
            root->keys.push_back(key);
        }
        else {
            if (root->keys.size() == m - 1) {
                Node* newRoot = new Node();
                newRoot->children.push_back(root);
                splitChild(newRoot, 0);
                root = newRoot;
            }
            insertNonFull(root, key);
        }
    }

    // 刪除功能
    void deleteKey(int key) {
        if (root) {
            deleteFromNode(root, key);
            if (root->keys.empty()) {
                // 如果根節點是空的，則需要處理根節點
                Node* temp = root;
                root = root->children.empty() ? nullptr : root->children[0];
                delete temp;
            }
        }
    }

    // 打印樹狀結構 (層級式)
    void printTree() {
        if (!root) {
            cout << "樹是空的。" << endl;
            return;
        }

        queue<pair<Node*, int>> q;
        q.push({ root, 0 });
        int currentLevel = 0;

        cout << "m-way Search Tree:" << endl;
        while (!q.empty()) {
            Node* node = q.front().first;
            int level = q.front().second;
            q.pop();

            if (level > currentLevel) {
                cout << endl;
                currentLevel = level;
            }

            cout << "[ ";
            for (int key : node->keys) {
                cout << key << " ";
            }
            cout << "] ";

            for (Node* child : node->children) {
                if (child) q.push({ child, level + 1 });
            }
        }
        cout << endl;
    }
};

int main() {
    int m;
    cout << "請輸入 m-way 搜尋樹的階數 m: ";
    cin >> m;

    MWayTree tree(m);

    cout << "請依序輸入整數值 (輸入 -1 結束):\n";
    while (true) {
        int value;
        cin >> value;
        if (value == -1) break;
        tree.insert(value);
    }

    cout << "\n生成的 m-way 搜尋樹:\n";
    tree.printTree();

    cout << "\n請輸入要刪除的整數值 (輸入 -1 結束):\n";
    while (true) {
        int value;
        cin >> value;
        if (value == -1) break;
        tree.deleteKey(value);
        cout << "\n刪除後的樹狀結構:\n";
        tree.printTree();
    }

    return 0;
}
