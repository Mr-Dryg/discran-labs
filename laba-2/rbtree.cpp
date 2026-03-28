#include <string>

struct Node {
    enum Color {
        black,
        red
    };

    std::string key;
    unsigned long long value;
    Color color;
    Node* left;
    Node* right;
    Node* parent;

    Node(const std::string& key, unsigned long long value, Node* parent, Color color)
        : key(key), value(value), color(color), left(nullptr), right(nullptr), parent(parent) {}
};

struct Data {
    enum Status {
        ok,
        exist,
        noSuchWord
    };

    Status status;
    unsigned long long value;
};

class RBTree {
    Node* root;

private:
    Data::Status add(Node*& node, const std::string& key, unsigned long long value) {
        if (key < node->key) {
            if (node->left == nullptr) {
                node->left = new Node(key, value, node, Node::red);
                // balance();
                return Data::ok;
            }
            return add(node->left, key, value);
        }
        else if (node->key < key) {
            if (node->right == nullptr) {
                node->right = new Node(key, value, node, Node::red);
                // balance();
                return Data::ok;
            }
            return add(node->right, key, value);
        }

        return Data::exist;
    }

    void del(Node* node) {
        // delete node and balance()
    }

    Node* find(Node* node, const std::string& key) {
        if (node == nullptr || key == node->key) {
            return node;
        }
        if (key < node->key) {
            return find(node->left, key);
        }
        else {
            return find(node->right, key);
        }
    }

public:
    RBTree(void) : root(nullptr) {}

    Data::Status add(const std::string& key, unsigned long long value) {
        if (root == nullptr) { // tree is empty
            root = new Node(key, value, nullptr, Node::black);
            return Data::ok;
        }
        return add(root, key, value);
    }

    Data::Status del(const std::string& key) {
        Node* node = find(root, key);
        if (node == nullptr) {
            return Data::noSuchWord;
        }
        del(node);
        return Data::ok;
    }

    Data find(const std::string& key) {
        Node* node = find(root, key);
        if (node == nullptr) {
            return Data{.status = Data::noSuchWord};
        }
        return Data{.status = Data::ok, .value = node->value};
    }
};