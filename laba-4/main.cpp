#include <algorithm>
#include <queue>
#include <unordered_map>
#include <vector>

class Trie {
    struct Node {
        std::unordered_map<unsigned int, Node*> next;  // try map to dicrease time/memmory
        size_t pattern_id = 0;
        Node* fail = nullptr;
        Node* out = nullptr;

        ~Node() {
            for (auto& [_, child] : next) {
                delete child;
            }
        }
    };

    struct Match {
        size_t line;
        size_t pos;
        size_t pattern_id;
    };

    Node* root;
    Node* cur;
    std::vector<size_t> pattern_lengths;
    std::vector<Match> mathes;
    std::vector<size_t> line_starts;

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

                if (node->fail->pattern_id != 0) {
                    node->out = node->fail;
                }
                else if (node->fail->out) {
                    node->out = node->fail->out;
                }
            }
        }
    }

    std::pair<size_t, size_t> getLineAndColumn(size_t end_pos, size_t pattern_length) {
        size_t start_pos = end_pos - pattern_length + 1;
        auto it = std::upper_bound(line_starts.begin(), line_starts.end(), start_pos);
        size_t line = std::distance(line_starts.begin(), it);
        size_t line_start = *(--it);
        return {line, start_pos - line_start + 1};
    }

    void saveMatch(Node* node) {
        auto [line, pos] = getLineAndColumn(
            *(--line_starts.end()),
            pattern_lengths[node->pattern_id - 1]
        );
        mathes.emplace_back(line, pos, node->pattern_id);
    }

public:
    Trie(const std::vector<std::vector<unsigned int>>& patterns) {
        pattern_lengths.resize(patterns.size());

        root = new Node;
        cur = root;
        for (size_t i = 0; i < patterns.size(); ++i) {
            Node* cur_node = root;
            for (const auto& symbol : patterns[i]) {
                if (!cur_node->next.contains(symbol)) {
                    cur_node->next[symbol] = new Node;
                }
                cur_node = cur_node->next[symbol];
            }
            cur_node->pattern_id = i + 1;
            pattern_lengths[i] = patterns[i].size();
        }
        buildLinks();
    }

    ~Trie() {
        delete root;
    }

    void feedSymbol(unsigned int c, size_t pos) {
        if (pos == 1) {
            line_starts.push_back(0);
            if (line_starts.size() > 1) {
                auto last = --line_starts.end();
                *last += *(last - 1);
            }
        }
        auto last = --line_starts.end();
        *last += pos;

        while (!cur->next.contains(c) && cur != root) {
            cur = cur->fail;
        }
        if (cur->next.contains(c)) {
            cur = cur->next[c];
        }
        else {
            cur = root;
        }

        if (cur->pattern_id != 0) {
            saveMatch(cur);
        }

        Node* out = cur->out;
        while (out) {
            saveMatch(out);
            out = out->out;
        }
    }

    std::vector<Match> getResults(void) {
        return mathes;
    }
};

int main(void) {
    
}