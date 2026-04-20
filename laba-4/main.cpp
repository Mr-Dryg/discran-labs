#include <algorithm>
#include <iostream>
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
    size_t absolute_pos;
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

    void saveMatch(size_t absolute_end, size_t pattern_id) {
        auto [line, pos] = getLineAndColumn(
            absolute_end,
            pattern_lengths[pattern_id - 1]
        );
        mathes.emplace_back(line, pos, pattern_id);
    }

public:
    Trie(const std::vector<std::vector<unsigned int>>& patterns) {
        pattern_lengths.resize(patterns.size());
        absolute_pos = 0;
        line_starts.push_back(1);

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

    void feedNewline() {
        line_starts.push_back(absolute_pos + 1);
        return;
    }

    void feedSymbol(const unsigned int c) {
        ++absolute_pos;

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
            saveMatch(absolute_pos, cur->pattern_id);
        }

        Node* out = cur->out;
        while (out) {
            saveMatch(absolute_pos, out->pattern_id);
            out = out->out;
        }
    }

    std::vector<Match> getResults(void) {
        return mathes;
    }
};

int main(void) {
    std::vector<unsigned int> p1 = {1};
    std::vector<unsigned int> p2 = {02};
    std::vector<std::vector<unsigned int>> patterns = {p1, p2};

    Trie trie(patterns);

    std::vector<std::vector<unsigned int>> text = {
        {0001},
        {1},
        {02},
        {2}
    };

    for (const auto& line : text) {
        for (const auto& word : line) {
            trie.feedSymbol(word);
        }
        trie.feedNewline();
    }

    for (auto match : trie.getResults()) {
        std::cout << match.line << ", " << match.pos;
        if (patterns.size() > 1) {
            std::cout << ", " << match.pattern_id;
        }
        std::cout << '\n';
    }
}