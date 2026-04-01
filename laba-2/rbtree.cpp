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
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;

        Node(const std::string& key, unsigned long long value, Node* parent, Color color)
            : key(key), value(value), color(color), parent(parent) {}

        bool isLeftSon() {
            return parent && this == parent->left;
        }

        Node* grandpa() {
            if (!parent) return nullptr;
            return parent->parent;
        }

        Node* bro() {
            if (!parent) return  nullptr;
            if (this->isLeftSon()) {
                return parent->right;
            }
            return parent->left;
        }

        Node* uncle() {
            return parent->bro();
        }
    };

    Node* root;

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    Node* leftRotation(Node* node) {
        if (!node->right) return node;

        Node* subroot = node->right;
        Node* mv_node = subroot->left;

        if (node->parent) {
            (node->isLeftSon() ? node->parent->left : node->parent->right) = subroot;
        }
        else {
            root = subroot;
        }
        subroot->parent = node->parent;
        subroot->left = node;
        node->parent = subroot;
        node->right = mv_node;
        if (mv_node) mv_node->parent = node;

        return subroot;
    }

    Node* rightRotation(Node* node) {
        if (!node->left) return node;

        Node* subroot = node->left;
        Node* mv_node = subroot->right;

        if (node->parent) {
            (node->isLeftSon() ? node->parent->left : node->parent->right) = subroot;
        }
        else {
            root = subroot;
        }
        subroot->parent = node->parent;
        subroot->right = node;
        node->parent = subroot;
        node->left = mv_node;
        if (mv_node) mv_node->parent = node;

        return subroot;
    }

    void insertBalance(Node* node) {
        while (node->parent && node->parent->color == Node::red) {
            Node* uncle = node->uncle();
            Node* grandpa = node->grandpa();

            if (uncle && uncle->color == Node::red) {
                node->parent->color = uncle->color = Node::black;
                grandpa->color = Node::red;
            }
            else if (!uncle || (uncle && uncle->color == Node::black)) {
                if (node->parent->isLeftSon() && !node->isLeftSon()) {
                    node = leftRotation(node->parent);
                }
                else if (!node->parent->isLeftSon() && node->isLeftSon()) {
                    node = rightRotation(node->parent);
                }
                else {
                    node = node->parent;
                }

                node->color = Node::black;
                grandpa->color = Node::red;

                if (node->isLeftSon()) {
                    grandpa = rightRotation(grandpa);
                }
                else {
                    grandpa = leftRotation(grandpa);
                }
            }
            node = grandpa;
        }
        root->color = Node::black;
    }

Data::Status _insert(Node* node, const std::string& key, unsigned long long value) {
    if (key == node->key) {
        return Data::exist;
    }
    
    Node*& next_node = (key < node->key) ? node->left : node->right;
    
    if (next_node) {
        return _insert(next_node, key, value);
    }
    
    next_node = new Node(key, value, node, Node::red);
    if (node->color == Node::red) {
        insertBalance(next_node);
    }
    return Data::ok;
}

    void _remove(Node* node) {
        // remove node and removeBalance()
    }

    Node* _find(Node* node, const std::string& key) {
        if (!node || key == node->key) {
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

    Data::Status insert(const std::string& key, unsigned long long value) {
        if (!root) {
            root = new Node(key, value, nullptr, Node::black);
            return Data::ok;
        }
        return _insert(root, key, value);
    }

    Data::Status remove(const std::string& key) {
        Node* node = _find(root, key);
        if (!node) {
            return Data::noSuchWord;
        }
        _remove(node);
        return Data::ok;
    }

    Data find(const std::string& key) {
        Node* node = _find(root, key);
        if (!node) {
            return Data{.status = Data::noSuchWord};
        }
        return Data{.status = Data::ok, .value = node->value};
    }
};