#include <algorithm>
#include <cstddef>
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
                std::cout << " ?";
            }
            else {
                std::cout << " " << symbol.value;
            }
        }
        return os;
    }
};

struct Node {
    std::unordered_map<unsigned int, Node*> next;  // try map to dicrease time/memmory
    size_t fragment_id = 0;
    Node* fail = nullptr;
    Node* out = nullptr;

    bool isEnd() const {
        return fragment_id != 0;
    }

    ~Node() {
        for (auto& [_, child] : next) {
            delete child;
        }
    }
};

struct FragmentInfo {
    size_t pattern_id;
    size_t position;
    size_t length;

    FragmentInfo(size_t p_id, size_t pos, size_t len) : pattern_id(p_id), position(pos), length(len) {}
};

class Trie {
    Node* root;
    std::vector<FragmentInfo> fragments_info;

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

    void addFragment(const std::vector<unsigned int>& fragment, size_t f_id, size_t p_id, size_t pos) {
        fragments_info.emplace_back(p_id, pos, fragment.size());
        Node* cur_node = root;
        for (const auto& symbol : fragment) {
            if (!cur_node->next.contains(symbol)) {
                cur_node->next[symbol] = new Node;
            }
            cur_node = cur_node->next[symbol];
        }
        if (cur_node->fragment_id == 0) {
            cur_node->fragment_id = f_id;
        }
    }

public:
    Trie(const std::vector<Pattern>& patterns) {
        fragments_info.reserve(patterns.size());
        root = new Node;

        size_t f_id = 0;

        for (size_t p_id = 1; p_id <= patterns.size(); ++p_id) {
            size_t position = 0;
            std::vector<unsigned int> fragment;

            for (const auto& symbol : patterns[p_id - 1].symbols) {
                if (symbol.isJoker) {
                    if (!fragment.empty()) {
                        addFragment(fragment, ++f_id, p_id, position);
                        fragment.clear();
                    }
                    ++position;
                }
                else {
                    fragment.push_back(symbol.value);
                }
            }
            if (!fragment.empty()) {
                addFragment(fragment, ++f_id, p_id, position);
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

    size_t getFragmentLength(size_t id) const {
        return fragments_info[id - 1].length;
    }

    size_t getFragmentsNumber() const {
        return fragments_info.size();
    }
};

class Scanner {
    const Trie& trie;
    const Node* cur;

    std::vector<std::vector<size_t>> mathes;
    size_t absolute_pos;
    std::vector<size_t> line_starts;

    void saveMatch(size_t absolute_end, const Node* node) {
        size_t abs_pos = absolute_end - trie.getFragmentLength(node->fragment_id) + 1;
        mathes[node->fragment_id - 1].push_back(abs_pos);
    }

public:
    Scanner(const Trie& trie) : trie(trie) {
        cur = trie.getRoot();
        mathes.resize(trie.getFragmentsNumber());
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

    const std::vector<std::vector<size_t>>& getResults(void) {
        return mathes;
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

    const std::vector<std::vector<size_t>>& mathes = scanner.getResults();
    for (int f_id = 1; f_id <= mathes.size(); ++f_id) {
        for (auto& abs_pos : mathes[f_id - 1]) {
            // auto [line, pos] = scanner.getLineAndColumn(abs_pos);
            // std::cout << line << ", " << pos;
            std::cout << abs_pos;
            // if (.size() > 1) {
            std::cout << ", " << f_id;
            // }
            std::cout << '\n';
        }
    }
}