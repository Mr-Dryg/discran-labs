#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

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

class Trie {
    Node* root;
    std::vector<size_t> pattern_lengths;

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

public:
    Trie(const std::vector<std::vector<unsigned int>>& patterns) {
        pattern_lengths.resize(patterns.size());

        root = new Node;
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

    Node* getRoot() const {
        return root;
    }

    size_t getPatternLength(size_t id) const {
        return pattern_lengths[id - 1];
    }

    size_t getPatternNumber() const {
        return pattern_lengths.size();
    }
};

class Scanner {
    const Trie& trie;
    const Node* cur;

    std::vector<std::vector<size_t>> mathes;
    size_t absolute_pos;
    std::vector<size_t> line_starts;

    void saveMatch(size_t absolute_end, size_t pattern_id) {
        size_t abs_pos = absolute_end - trie.getPatternLength(pattern_id) + 1;
        mathes[pattern_id - 1].push_back(abs_pos);
    }

public:
    Scanner(const Trie& trie) : trie(trie) {
        cur = trie.getRoot();
        mathes.resize(trie.getPatternNumber());
        absolute_pos = 0;
        line_starts.push_back(1);
    }

    void feedSymbol(const unsigned int symbol) {
        ++absolute_pos;

        while (!cur->next.contains(symbol) && cur != trie.getRoot()) {
            cur = cur->fail;
        }
        if (cur->next.contains(symbol)) {
            cur = cur->next.find(symbol)->second;
        }
        else {
            cur = trie.getRoot();
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

    void feedNewline() {
        line_starts.push_back(absolute_pos + 1);
        return;
    }

    std::pair<size_t, size_t> getLineAndColumn(size_t start_pos) {
        auto it = std::upper_bound(line_starts.begin(), line_starts.end(), start_pos);
        size_t line = std::distance(line_starts.begin(), it);
        size_t line_start = *(--it);
        return {line, start_pos - line_start + 1};
    }

    const std::vector<std::vector<size_t>>& getResults(void) {
        return mathes;
    }
};

int main(void) {
    std::vector<unsigned int> p1 = {1};
    std::vector<unsigned int> p2 = {02};
    std::vector<std::vector<unsigned int>> patterns = {p1, p2};

    Trie trie(patterns);
    Scanner scanner(trie);

    std::vector<std::vector<unsigned int>> text = {
        {0001},
        {1},
        {02},
        {2}
    };

    for (const auto& line : text) {
        for (const auto& word : line) {
            scanner.feedSymbol(word);
        }
        scanner.feedNewline();
    }

    const std::vector<std::vector<size_t>>& mathes = scanner.getResults();
    for (int pattern_id = 1; pattern_id <= patterns.size(); ++pattern_id) {
        for (auto& abs_pos : mathes[pattern_id - 1]) {
            // auto [line, pos] = scanner.getLineAndColumn(abs_pos);
            // std::cout << line << ", " << pos;
            std::cout << abs_pos;
            if (patterns.size() > 1) {
                std::cout << ", " << pattern_id;
            }
            std::cout << '\n';
        }
    }
}