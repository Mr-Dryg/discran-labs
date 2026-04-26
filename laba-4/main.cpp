#include <algorithm>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
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
        // std::cout << "FEED: " << symbol << '\n';
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

class Pattern {
    struct Symbol {
        unsigned int value;
        bool isJoker = false;

        Symbol(unsigned int value, bool isJoker) : value(value), isJoker(isJoker) {}
    };

    std::vector<Symbol> symbols;

public:
    Pattern(std::istream& is) {
        unsigned int num;
        std::string token;
        
        while (is >> token) {
            if (token == "?") {
                symbols.emplace_back(0, true);
            } else {
                try {
                    num = std::stoul(token);
                    symbols.emplace_back(num, false);
                } catch (...) {
                    throw std::runtime_error("Invalid token: \"" + token + "\"");
                }
            }
        }
    }

    std::vector<std::vector<unsigned int>> getPattern() const{
        std::vector<std::vector<unsigned int>> res;
        res.resize(1);
        int i = 0;
        for (const auto& symbol : symbols) {
            if (!symbol.isJoker) {
                res[i].push_back(symbol.value);
            }
            else {
                res.resize(++i + 1);
            }
        }
        return res;
    }

    friend std::ostream& operator<<(std::ostream& os, Pattern& pattern) {
        for (auto& symbol : pattern.symbols) {
            if (symbol.isJoker) {
                std::cout << " ?";
            }
            else {
                std::cout << " " << symbol.value;
            }
        }
        return os;
    }
};

int main(void) {
    std::vector<Pattern> patterns;

    std::string line;
    std::istringstream iss;

    std::getline(std::cin, line);
    iss.str(std::move(line));
    patterns.emplace_back(iss);
    iss.clear();

    std::vector<std::string> buffer;
    while (std::getline(std::cin, line) && line != "") {
        buffer.push_back(std::move(line));
    }

    // std::cout << "BUFFER:\n";
    // for (auto& line : buffer) {
    //     std::cout << line << "\n";
    // }

    if (!std::cin.eof() && line == "") {
        for (auto& line : buffer) {
            iss.str(std::move(line));
            patterns.emplace_back(iss);
            iss.clear();
        }
        buffer.clear();
    }

    // for (int i = 0; i < patterns.size(); ++i) {
    //     std::cout << "PATTERNS[" << i << "]: " << patterns[i] << "\n";
    // }

    std::vector<std::vector<unsigned int>> patterns_without_jokers;
    for (const auto& pattern : patterns) {
        auto p = pattern.getPattern();
        patterns_without_jokers.insert(
            patterns_without_jokers.end(),
            std::make_move_iterator(p.begin()),
            std::make_move_iterator(p.end())
        );
    }

    // std::cout << "patterns_without_jokers:\n";
    // for (auto& word : patterns_without_jokers) {
    //     for (auto& symbol: word) {
    //         std::cout << " " << symbol;
    //     }
    //     std::cout << "\n";
    // }

    Trie trie(patterns_without_jokers);
    Scanner scanner(trie);

    // std::cout << "BUFFER IS EMPTY: " << buffer.empty() << "\n";

    unsigned int num = 5;
    if (!buffer.empty()) {
        for (auto& line : buffer) {
            // std::cout << "Line: " << line << "\n";
            iss.str(std::move(line));
            // iss >> num;
            while (iss >> num) {
            // std::cout << "num: " << num;
                scanner.feedSymbol(num);
            }
            iss.clear();
            scanner.feedNewline();
        }
        buffer.clear();
    }

    // std::cout << "BUFFER IS EMPTY: " << buffer.empty() << "\n";

    while (std::getline(std::cin, line)) {
        iss.str(std::move(line));
        while (iss >> num) {
            scanner.feedSymbol(num);
        }
        iss.clear();
        scanner.feedNewline();
    }

    const std::vector<std::vector<size_t>>& mathes = scanner.getResults();

    // std::cout << "mathes IS EMPTY: " << mathes.empty() << "\n";

    for (int pattern_id = 1; pattern_id <= patterns_without_jokers.size(); ++pattern_id) {
        // std::cout << "mathes[" << pattern_id - 1 << "] IS EMPTY: " << mathes[pattern_id - 1].empty() << "\n";
        for (auto& abs_pos : mathes[pattern_id - 1]) {
            // auto [line, pos] = scanner.getLineAndColumn(abs_pos);
            // std::cout << line << ", " << pos;
            std::cout << abs_pos;
            if (patterns_without_jokers.size() > 1) {
                std::cout << ", " << pattern_id;
            }
            std::cout << '\n';
        }
    }
}