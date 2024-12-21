#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
#include <algorithm>

using namespace std;

// 節點結構 (適用於 B-tree)
struct Node {
	vector<int> keys;           // 儲存關鍵值
	vector<Node*> children;     // 子節點指標
	bool leaf;                  // 是否為葉子節點

	Node(bool isLeaf) : leaf(isLeaf) {}
};

// B-tree 類別
class BTree {
private:
	int t;              // B-tree 的最小度數
	Node* root;         // 樹的根節點

	// 插入輔助函式 (非滿節點)
	void insertNonFull(Node* node, int key) {
		int i = node->keys.size() - 1;

		if (node->leaf) {
			// 插入到葉子節點
			node->keys.push_back(0);
			while (i >= 0 && key < node->keys[i]) {
				node->keys[i + 1] = node->keys[i];
				i--;
			}
			node->keys[i + 1] = key;
		}
		else {
			// 找到適當的子節點
			while (i >= 0 && key < node->keys[i]) {
				i--;
			}
			i++;
			if (node->children[i]->keys.size() == 2 * t - 1) {
				// 如果子節點滿了，先分裂
				splitChild(node, i);
				if (key > node->keys[i]) {
					i++;
				}
			}
			insertNonFull(node->children[i], key);
		}
	}

	// 分裂子節點
	void splitChild(Node* parent, int index) {
		Node* fullChild = parent->children[index];
		Node* newChild = new Node(fullChild->leaf);

		int mid = t - 1;
		parent->keys.insert(parent->keys.begin() + index, fullChild->keys[mid]);
		parent->children.insert(parent->children.begin() + index + 1, newChild);

		// 將滿節點的右半部分移到新節點
		newChild->keys.assign(fullChild->keys.begin() + mid + 1, fullChild->keys.end());
		fullChild->keys.resize(mid);

		if (!fullChild->leaf) {
			newChild->children.assign(fullChild->children.begin() + t, fullChild->children.end());
			fullChild->children.resize(t);
		}
	}

	// 刪除輔助函式
	bool deleteFromNode(Node* node, int key) {
		int idx = findKey(node, key);

		if (idx != -1) {
			// 找到關鍵值並刪除
			if (node->leaf) {
				node->keys.erase(node->keys.begin() + idx);
			}
			else {
				deleteInternalKey(node, idx);
			}
			return true;
		}
		else if (!node->leaf) {
			// 關鍵值不在當前節點，遞歸刪除
			return deleteFromChild(node, key);
		}
		return false;
	}

	// 查找關鍵值在節點中的索引
	int findKey(Node* node, int key) {
		for (size_t i = 0; i < node->keys.size(); i++) {
			if (node->keys[i] == key) return i;
		}
		return -1;
	}

	// 刪除內部鍵值
	void deleteInternalKey(Node* node, int idx) {
		int key = node->keys[idx];
		Node* leftChild = node->children[idx];
		Node* rightChild = node->children[idx + 1];

		if (leftChild->keys.size() >= t) {
			int predecessor = getMaxKey(leftChild);
			node->keys[idx] = predecessor;
			deleteFromNode(leftChild, predecessor);
		}
		else if (rightChild->keys.size() >= t) {
			int successor = getMinKey(rightChild);
			node->keys[idx] = successor;
			deleteFromNode(rightChild, successor);
		}
		else {
			mergeChildren(node, idx);
			deleteFromNode(leftChild, key);
		}
	}

	// 合併子節點
	void mergeChildren(Node* parent, int idx) {
		Node* leftChild = parent->children[idx];
		Node* rightChild = parent->children[idx + 1];

		leftChild->keys.push_back(parent->keys[idx]);
		leftChild->keys.insert(leftChild->keys.end(), rightChild->keys.begin(), rightChild->keys.end());

		if (!leftChild->leaf) {
			leftChild->children.insert(leftChild->children.end(), rightChild->children.begin(), rightChild->children.end());
		}

		parent->keys.erase(parent->keys.begin() + idx);
		parent->children.erase(parent->children.begin() + idx + 1);

		delete rightChild;
	}

	// 從子節點刪除鍵值
	bool deleteFromChild(Node* parent, int key) {
		int idx = 0;
		while (idx < parent->keys.size() && key > parent->keys[idx]) {
			idx++;
		}
		Node* child = parent->children[idx];

		if (child->keys.size() < t) {
			rebalanceChild(parent, idx);
		}
		return deleteFromNode(child, key);
	}

	// 重組子節點
	void rebalanceChild(Node* parent, int idx) {
		Node* child = parent->children[idx];

		if (idx > 0 && parent->children[idx - 1]->keys.size() >= t) {
			borrowFromLeft(parent, idx);
		}
		else if (idx < parent->keys.size() && parent->children[idx + 1]->keys.size() >= t) {
			borrowFromRight(parent, idx);
		}
		else {
			if (idx > 0) {
				mergeChildren(parent, idx - 1);
			}
			else {
				mergeChildren(parent, idx);
			}
		}
	}

	// 從左邊借鍵值
	void borrowFromLeft(Node* parent, int idx) {
		Node* child = parent->children[idx];
		Node* leftSibling = parent->children[idx - 1];

		child->keys.insert(child->keys.begin(), parent->keys[idx - 1]);
		parent->keys[idx - 1] = leftSibling->keys.back();
		leftSibling->keys.pop_back();

		if (!leftSibling->leaf) {
			child->children.insert(child->children.begin(), leftSibling->children.back());
			leftSibling->children.pop_back();
		}
	}

	// 從右邊借鍵值
	void borrowFromRight(Node* parent, int idx) {
		Node* child = parent->children[idx];
		Node* rightSibling = parent->children[idx + 1];

		child->keys.push_back(parent->keys[idx]);
		parent->keys[idx] = rightSibling->keys.front();
		rightSibling->keys.erase(rightSibling->keys.begin());

		if (!rightSibling->leaf) {
			child->children.push_back(rightSibling->children.front());
			rightSibling->children.erase(rightSibling->children.begin());
		}
	}

	// 取得最大鍵值
	int getMaxKey(Node* node) {
		while (!node->leaf) {
			node = node->children.back();
		}
		return node->keys.back();
	}

	// 取得最小鍵值
	int getMinKey(Node* node) {
		while (!node->leaf) {
			node = node->children.front();
		}
		return node->keys.front();
	}

public:
	BTree(int t) : t(t), root(nullptr) {}

	// 插入鍵值
	void insert(int key) {
		if (!root) {
			root = new Node(true);
			root->keys.push_back(key);
		}
		else {
			if (root->keys.size() == 2 * t - 1) {
				Node* newRoot = new Node(false);
				newRoot->children.push_back(root);
				splitChild(newRoot, 0);
				root = newRoot;
			}
			insertNonFull(root, key);
		}
	}

	// 刪除鍵值
	void deleteKey(int key) {
		if (root) {
			deleteFromNode(root, key);
			if (root->keys.empty()) {
				Node* temp = root;
				root = root->leaf ? nullptr : root->children[0];
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

		cout << "B-tree:" << endl;
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
	int t = 2; // B-tree 的最小度數
	BTree btree(t);

	while (true) {
		cout << "\n=== B-tree 操作選單 ===\n";
		cout << "1. 新增鍵值\n";
		cout << "2. 刪除鍵值\n";
		cout << "3. 打印 B-tree\n";
		cout << "4. 結束程序\n";
		cout << "請選擇操作: ";

		int choice;
		cin >> choice;

		if (choice == 1) {
			cout << "輸入要新增的鍵值 (輸入 -1 結束): ";
			while (true) {
				int key;
				cin >> key;
				if (key == -1) {
					cout << "結束新增操作。\n";
					break;
				}
				btree.insert(key);
				cout << "鍵值 " << key << " 已新增。\n";
			}
		}
		else if (choice == 2) {
			cout << "輸入要刪除的鍵值: ";
			int key;
			cin >> key;
			btree.deleteKey(key);
			cout << "鍵值 " << key << " 已刪除。\n";
		}
		else if (choice == 3) {
			cout << "當前 B-tree 結構:\n";
			btree.printTree();
		}
		else if (choice == 4) {
			cout << "程序結束。\n";
			break;
		}
		else {
			cout << "無效的選擇，請重新輸入。\n";
		}
	}

	return 0;
}
