#include <algorithm>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

struct Symbol {
    unsigned int value;
    bool isJoker = false;

    Symbol(unsigned int value, bool isJoker) : value(value), isJoker(isJoker) {}
};

struct Pattern {
    std::vector<Symbol> symbols;

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

    friend std::ostream& operator<<(std::ostream& os, Pattern& pattern) {
        for (auto& symbol : pattern.symbols) {
            if (symbol.isJoker) {
                os << " ?";
            }
            else {
                os << " " << symbol.value;
            }
        }
        return os;
    }
};

struct FragmentInfo {
    size_t pattern_id;
    size_t position;
    size_t length;

    FragmentInfo(size_t p_id, size_t pos, size_t len) : pattern_id(p_id), position(pos), length(len) {}
};

struct Node {
    std::unordered_map<unsigned int, Node*> next;  // try map to dicrease time/memmory
    std::vector<FragmentInfo> fragments;
    Node* fail = nullptr;
    Node* out = nullptr;

    bool isEnd() const {
        return !fragments.empty();
    }

    ~Node() {
        for (auto& [_, child] : next) {
            delete child;
        }
    }
};

class Trie {
    Node* root;
    std::vector<size_t> pattern_fragments;

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

                if (node->fail->isEnd()) {
                    node->out = node->fail;
                }
                else if (node->fail->out) {
                    node->out = node->fail->out;
                }
            }
        }
    }

    void addFragment(const std::vector<unsigned int>& fragment, size_t p_id, size_t pos) {
        ++pattern_fragments[p_id - 1];
        Node* cur_node = root;
        for (const auto& symbol : fragment) {
            if (!cur_node->next.contains(symbol)) {
                cur_node->next[symbol] = new Node;
            }
            cur_node = cur_node->next[symbol];
        }
        cur_node->fragments.emplace_back(p_id, pos, fragment.size());
    }

public:
    Trie(const std::vector<Pattern>& patterns) {
        pattern_fragments.resize(patterns.size());
        root = new Node;

        for (size_t p_id = 1; p_id <= patterns.size(); ++p_id) {
            size_t position = 0;
            std::vector<unsigned int> fragment;

            for (const auto& symbol : patterns[p_id - 1].symbols) {
                if (symbol.isJoker) {
                    if (!fragment.empty()) {
                        addFragment(fragment, p_id, position);
                        fragment.clear();
                    }
                }
                else {
                    fragment.push_back(symbol.value);
                }
                ++position;
            }
            if (!fragment.empty()) {
                addFragment(fragment, p_id, position);
            }
        }
        buildLinks();
    }

    ~Trie() {
        delete root;
    }

    Node* getRoot() const {
        return root;
    }

    size_t getPatternFragments(size_t p_id) const {
        return pattern_fragments[p_id - 1];
    }
};

struct Match {
    size_t abs_start;
    size_t votes = 1;

    Match(size_t abs_start) : abs_start(abs_start) {}
};

class Scanner {
    const Trie& trie;
    const Node* cur;

    std::unordered_map<size_t, std::vector<Match>> matches;
    size_t absolute_pos;
    std::vector<size_t> line_starts;

    void saveMatch(size_t absolute_end, const Node* node) {
        // std::cout << "SAVING MATCH: " << absolute_end << '\n';
        for (const auto& f : node->fragments) {
            size_t abs_pos = absolute_end - f.length + 1;
            // std::cout << "f.position: " << f.position << '\n';
            if (f.position == 1) {
                if (!matches.contains(f.pattern_id)) {
                    matches[f.pattern_id].emplace_back(abs_pos);
                }
                else {
                    if (matches[f.pattern_id].back().votes != trie.getPatternFragments(f.pattern_id)) {
                        matches[f.pattern_id].back() = {abs_pos};
                    }
                    else {
                        matches[f.pattern_id].emplace_back(abs_pos);
                    }
                }
            }
            else {
                if (matches.contains(f.pattern_id) && !matches[f.pattern_id].empty()) {
                    if (absolute_end - f.position + 1 == matches[f.pattern_id].back().abs_start) {
                        ++matches[f.pattern_id].back().votes;
                    }
                    else {
                        matches[f.pattern_id].pop_back();
                    }
                }
            }
        }
    }

public:
    Scanner(const Trie& trie) : trie(trie) {
        cur = trie.getRoot();
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

        if (cur->isEnd()) {
            saveMatch(absolute_pos, cur);
        }

        Node* out = cur->out;
        while (out) {
            saveMatch(absolute_pos, out);
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

    const std::unordered_map<size_t, std::vector<Match>>& getResults(void) {
        for (auto& [pattern_id, v_m] : matches) {
            if (!v_m.empty() && v_m.back().votes != trie.getPatternFragments(pattern_id)) {
                v_m.pop_back();
            }
        }

        return matches;
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

    if (!std::cin.eof() && line == "") {
        for (auto& line : buffer) {
            iss.str(std::move(line));
            patterns.emplace_back(iss);
            iss.clear();
        }
        buffer.clear();
    }

    Trie trie(patterns);
    Scanner scanner(trie);

    unsigned int num;
    if (!buffer.empty()) {
        for (auto& line : buffer) {
            iss.str(std::move(line));
            while (iss >> num) {
                scanner.feedSymbol(num);
            }
            iss.clear();
            scanner.feedNewline();
        }
        buffer.clear();
    }

    while (std::getline(std::cin, line)) {
        iss.str(std::move(line));
        while (iss >> num) {
            scanner.feedSymbol(num);
        }
        iss.clear();
        scanner.feedNewline();
    }

    auto result = scanner.getResults();
    // std::cout << "result is empty: " << result.empty() << "\n";
    for (const auto& [pattern_id, matches] : result) {
        // std::cout << "pattern " << pattern_id << " was found: " << !matches.empty() << "\n";
        for (const auto& match : matches) {
            // auto [line, pos] = scanner.getLineAndColumn(abs_pos);
            // std::cout << line << ", " << pos;
            std::cout << match.abs_start;
            // if (.size() > 1) {
            std::cout << ", " << pattern_id;
            // }
            std::cout << '\n';
        }
    }
}