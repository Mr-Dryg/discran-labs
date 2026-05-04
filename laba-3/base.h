#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

enum Side {
    left,
    right
};

struct ReturnData {
    enum Status {
        ok,
        exist,
        noSuchWord
    } status;

    unsigned long long value;

    friend std::ostream& operator<<(std::ostream& os, Status& status) {
        switch (status) {
            case ok:
                std::cout << "OK";
                break;
            case exist:
                std::cout << "Exist";
                break;
            case noSuchWord:
                std::cout << "NoSuchWord";
                break;
        }
        return os;
    }
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

        Node(std::ifstream& file) {
            unsigned char color_byte;
            file.read(reinterpret_cast<char*>(&color_byte), sizeof(color_byte));
            if (file.fail()) throw std::runtime_error("Failed to read color");
            if (color_byte > 1) throw std::runtime_error("Invalid color value in file");
            color = static_cast<Color>(color_byte);
            
            size_t len;
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            if (file.fail()) throw std::runtime_error("Failed to read key length");
            
            if (len > 0) {
                key.resize(len);
                file.read(&key[0], len);
                if (file.fail()) throw std::runtime_error("Failed to read key");
            }
            else {
                key = "";
            }
            
            file.read(reinterpret_cast<char*>(&value), sizeof(value));
            if (file.fail()) throw std::runtime_error("Failed to read value");
        }
        
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
            if (isLeftSon()) {
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

        void save(std::ofstream& file) const {
            unsigned char color_byte = static_cast<unsigned char>(color);
            file.write(reinterpret_cast<const char*>(&color_byte), sizeof(color_byte));
            if (file.fail()) throw std::runtime_error("Failed to write color");
            
            size_t len = key.length();
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            if (file.fail()) throw std::runtime_error("Failed to write key length");
            
            file.write(key.c_str(), len);
            if (file.fail()) throw std::runtime_error("Failed to write key");
            
            file.write(reinterpret_cast<const char*>(&value), sizeof(value));
            if (file.fail()) throw std::runtime_error("Failed to write value");
        }
    };

    Node* root = nullptr;

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

    ReturnData::Status _insert(Node* node, const std::string& key, unsigned long long value) {
        if (key == node->key) {
            return ReturnData::exist;
        }
        
        Node*& new_node = (key < node->key) ? node->left : node->right;
        
        if (new_node) {
            return _insert(new_node, key, value);
        }
        
        new_node = new Node(key, value, node, Node::red);
        if (node->color == Node::red) {
            insertBalance(new_node);
        }
        return ReturnData::ok;
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
            removeBalance(
                parent, 
                node_is_left_son ? Side::left : Side::right
            );
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

    static void toUpper(std::string& str) {
        for (char& c : str) {
            if ('a' <= c && c <= 'z') {
                c += 'A' - 'a';
            }
        }
    }

    void _save(Node* node, std::ofstream& file) {
        if (!node) {
            file.write("$", 1);
            if (file.fail()) throw std::runtime_error("Failed to write null marker");
            return;
        }
        
        file.write("#", 1);
        if (file.fail()) throw std::runtime_error("Failed to write node marker");
        
        node->save(file);
        _save(node->left, file);
        _save(node->right, file);
    }

    Node* _load(std::ifstream& file) {
        char marker;
        file.read(&marker, 1);

        if (file.fail()) {
            throw std::runtime_error("Failed to read node marker");
        }
        
        if (marker == '$') {
            return nullptr;
        }
        
        if (marker != '#') {
            throw std::runtime_error("Invalid node marker in file");
        }
        
        Node* node = new Node(file);
        
        try {
            node->left = _load(file);
            node->right = _load(file);
        }
        catch (const std::exception& e) {
            destroy(node);
            throw;
        }

        if (node->left) {
            node->left->parent = node;
        }

        if (node->right) {
            node->right->parent = node;
        }
        
        return node;
    }

public:
    RBTree(void) : root(nullptr) {}

    ~RBTree() {
        destroy(root);
    }

    ReturnData::Status insert(std::string key, unsigned long long value) {
        toUpper(key);
        if (!root) {
            root = new Node(key, value, nullptr, Node::black);
            return ReturnData::ok;
        }
        return _insert(root, key, value);
    }

    ReturnData::Status remove(std::string key) {
        toUpper(key);
        Node* node = _find(root, key);
        if (!node) {
            return ReturnData::noSuchWord;
        }
        _remove(node);
        return ReturnData::ok;
    }

    ReturnData find(std::string key) {
        toUpper(key);
        Node* node = _find(root, key);
        if (!node) {
            return ReturnData{.status = ReturnData::noSuchWord};
        }
        return ReturnData{.status = ReturnData::ok, .value = node->value};
    }

    ReturnData::Status save(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (root) {
            _save(root, file);
        }
        
        return ReturnData::ok;
    }

    ReturnData::Status load(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);

        Node* new_root = nullptr;

        if (file.peek() != std::ifstream::traits_type::eof()) {
            new_root = _load(file);

            if (file.peek() != std::ifstream::traits_type::eof()) {
                destroy(new_root);
                throw std::runtime_error("Extra data at end of file");
            }
        }

        destroy(root);
        root = new_root;
        return ReturnData::ok;
    }
};