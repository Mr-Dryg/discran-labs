#include <stdexcept>
#include <functional>
#include <string>
#include <utility>

enum Side {
    left,
    right
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
    struct Node {
        enum Color {
            red,
            black
        };

        std::string key;
        unsigned long long value;
        Color color;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;

        Node(const std::string& key, unsigned long long value, Node* parent, Color color)
            : key(key), value(value), color(color), parent(parent) {}
        
        void update(const std::string& key, unsigned long long value) {
            this->key = key;
            this->value = value;
        }

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

        Node*& parentLink() {
            return isLeftSon() ? parent->left : parent->right;
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
            node->parentLink() = subroot;
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
            node->parentLink() = subroot;
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
        if (root) root->color = Node::black;
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

    Node* rotation(Node* base_node, Side side, bool replace_colors, Node* other) {
        if (replace_colors && other) {
            Node::Color base_node_color = base_node->color;
            base_node->color = other->color;
            other->color = base_node_color;
        }
        if (side == left) {
            return leftRotation(base_node);
        }
        else if (side == right) {
            return rightRotation(base_node);
        };
        throw std::invalid_argument("Invalid rotation side");
    }

    void removeBalance(Node* parent, Side side) {
        if (!parent) return;
        Node* node = side == left ? parent->left : parent->right;
        if (node && node->color == Node::red) {
            node->color = Node::black;
            if (root) root->color = Node::black;
            return;
        }

        Node* bro = side == left ? parent->right : parent->left;

        if (bro && bro->color == Node::red) {
            rotation(
                parent,
                side == left ? left : right,
                true,
                bro
            );
            bro = side == left ? parent->right : parent->left;
        }

        if (!bro || bro->color == Node::black) {
            if (
                !bro || (
                    (!bro->left || bro->left->color == Node::black) &&
                    (!bro->right || bro->right->color == Node::black)
                )
            ) {
                if (bro) bro->color = Node::red;
                if (parent->color == Node::black) {
                    removeBalance(parent->parent, parent->isLeftSon() ? left : right);
                }
                else {
                    parent->color = Node::black;
                }
                if (root) root->color = Node::black;
                return;
            }

            bool left_son_case = side == left &&
                                (bro->left && bro->left->color == Node::red) &&
                                (!bro->right || bro->right->color == Node::black);

            bool right_son_case = side == right &&
                                (bro->right && bro->right->color == Node::red) &&
                                (!bro->left || bro->left->color == Node::black);

            if (left_son_case || right_son_case) {
                rotation(
                    bro, 
                    side == left ? right : left,
                    true,
                    side == left ? bro->left : bro->right
                );
                bro = side == left ? parent->right : parent->left;
            }

            left_son_case = side == left &&
                            (bro->right && bro->right->color == Node::red);

            right_son_case = side == right &&
                             (bro->left && bro->left->color == Node::red);
            
            if (left_son_case || right_son_case) {
                (side == left ? bro->right : bro->left)->color = Node::black;
                rotation(
                    parent,
                    side == left ? left : right,
                    true,
                    bro
                );
            }
        }

        if (root) root->color = Node::black;
    }

    Node* minNode(Node* node) {
        for (; node && node->left; node = node->left) {}
        return node;
    }

    void _remove(Node* node) {
        if (node->left && node->right) {
            Node* node_to_rm = minNode(node->right);
            node->key = std::move(node_to_rm->key);
            node->value = std::move(node_to_rm->value);
            _remove(node_to_rm);
            return;
        }

        Node* parent = nullptr;
        Node* child = nullptr;
        Node::Color rm_color;
        bool node_is_left_son = node->isLeftSon();

        if (!node->left && !node->right) {
            parent = node->parent;
            rm_color = node->color;
        }
        else if (!node->left + !node->right == 1) {
            parent = node->parent;
            child = node->left ? node->left : node->right;
            rm_color = child->color;
        }

        if (!parent) {
            root = child;
        }
        else {
            node->parentLink() = child;
        }
        if (child) {
            child->parent = parent;
            child->color = node->color;
        }
        delete node;
        if (rm_color == Node::black) {
            // if (child) {
            //     removeBalance(child);
            // }
            // else {
            removeBalance(
                parent, 
                node_is_left_son ? Side::left : Side::right
            );
            // }
        }
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