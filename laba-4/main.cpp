#include <queue>
#include <unordered_map>
#include <vector>

class Trie {
    struct Node {
        std::unordered_map<unsigned int, Node*> next;  // try map to dicrease time/memmory
        size_t pattern_n = 0;
        Node* fail = nullptr;
        Node* out = nullptr;

        ~Node() {
            for (auto& [_, child] : next) {
                delete child;
            }
        }
    };

    Node* root;

    void buildLinks(void) {
        std::queue<Node*> nodes;
        nodes.push(root);

        root->fail = root;
        while (!nodes.empty()) {
            Node* parent = nodes.front();
            nodes.pop();

            for (const auto& [symbol, node] : parent->next) {
                nodes.push(node);

                if (parent == root) {
                    node->fail = root;
                }
                else {
                    Node* fail = parent->fail;
                    while (!fail->next.contains(symbol) && fail != root) {
                        fail = fail->fail;
                    }

                    if (fail->next.contains(symbol)) {
                        node->fail = fail->next[symbol];
                    }
                    else {
                        node->fail = fail;
                    }
                }

                if (node->fail->pattern_n != 0) {
                    node->out = node->fail;
                }
                else if (node->fail->out) {
                    node->out = node->fail->out;
                }
            }
        }
    }

public:
    Trie(const std::vector<std::vector<unsigned int>>& patterns) {
        root = new Node;
        for (size_t i = 0; i < patterns.size(); ++i) {
            Node* cur_node = root;
            for (const auto& symbol : patterns[i]) {
                if (!cur_node->next.contains(symbol)) {
                    cur_node->next[symbol] = new Node;
                }
                cur_node = cur_node->next[symbol];
            }
            cur_node->pattern_n = i + 1;
        }
        buildLinks();
    }

    ~Trie() {
        delete root;
    }

};

int main(void) {
    
}