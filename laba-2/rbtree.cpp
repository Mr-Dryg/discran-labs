#include <string>

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

        Node* _grandpa() {
            if (parent == nullptr) {
                return nullptr;
            }
            return parent->parent;
        }

        bool _isLeftSon() {
            return this == parent->left;
        }

        Node* _uncle() {
            Node* grandpa;
            if ((grandpa = this->_grandpa()) == nullptr) {
                return nullptr;
            }
            if (this->_isLeftSon()) {
                return grandpa->right;
            }
            return grandpa->left;
        }
    };

    Node* root;

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    Node* balance(Node*& node) {
        return nullptr;
    }

    Data::Status _add(Node*& node, const std::string& key, unsigned long long value) {
        if (key < node->key) {
            if (node->left == nullptr) {
                node->left = new Node(key, value, node, Node::red);
                balance(node->left);
                return Data::ok;
            }
            return _add(node->left, key, value);
        }
        else if (node->key < key) {
            if (node->right == nullptr) {
                node->right = new Node(key, value, node, Node::red);
                balance(node->right);
                return Data::ok;
            }
            return _add(node->right, key, value);
        }

        return Data::exist;
    }

    void _del(Node* node) {
        // delete node and balance()
    }

    Node* _find(Node* node, const std::string& key) {
        if (node == nullptr || key == node->key) {
            return node;
        }
        if (key < node->key) {
            return _find(node->left, key);
        }
        else {
            return _find(node->right, key);
        }
    }

public:
    RBTree(void) : root(nullptr) {}

    ~RBTree() {
        destroy(root);
    }

    Data::Status add(const std::string& key, unsigned long long value) {
        if (root == nullptr) {
            root = new Node(key, value, nullptr, Node::black);
            return Data::ok;
        }
        return _add(root, key, value);
    }

    Data::Status del(const std::string& key) {
        Node* node = _find(root, key);
        if (node == nullptr) {
            return Data::noSuchWord;
        }
        _del(node);
        return Data::ok;
    }

    Data find(const std::string& key) {
        Node* node = _find(root, key);
        if (node == nullptr) {
            return Data{.status = Data::noSuchWord};
        }
        return Data{.status = Data::ok, .value = node->value};
    }
};