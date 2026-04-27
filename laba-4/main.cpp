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
    size_t end_position;
    size_t length;

    FragmentInfo(size_t p_id, size_t end_pos, size_t len) : pattern_id(p_id), end_position(end_pos), length(len) {}
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

                if (node->fail->isEnd()) {
                    node->out = node->fail;
                }
                else if (node->fail->out) {
                    node->out = node->fail->out;
                }
            }
        }
    }

    void addFragment(const std::vector<unsigned int>& fragment, size_t p_id, size_t end_pos) {
        ++pattern_fragments[p_id - 1];
        Node* cur_node = root;
        for (const auto& symbol : fragment) {
            if (!cur_node->next.contains(symbol)) {
                cur_node->next[symbol] = new Node;
            }
            cur_node = cur_node->next[symbol];
        }
        cur_node->fragments.emplace_back(p_id, end_pos, fragment.size());
    }

public:
    Trie(const std::vector<Pattern>& patterns) {
        pattern_fragments.resize(patterns.size());
        root = new Node;
        for (size_t p_id = 1; p_id <= patterns.size(); ++p_id) {
            pattern_lengths.push_back(patterns[p_id - 1].symbols.size());
            size_t end_position = 0;
            std::vector<unsigned int> fragment;

            for (const auto& symbol : patterns[p_id - 1].symbols) {
                if (symbol.isJoker) {
                    if (!fragment.empty()) {
                        addFragment(fragment, p_id, end_position);
                        fragment.clear();
                    }
                }
                else {
                    fragment.push_back(symbol.value);
                }
                ++end_position;
            }
            if (!fragment.empty()) {
                addFragment(fragment, p_id, end_position);
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

    size_t getPatternLength(size_t p_id) const {
        return pattern_lengths[p_id - 1];;
    }

    size_t getPatternsNumber() const {
        return pattern_lengths.size();
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
        for (const auto& f : node->fragments) {
            long long abs_start = absolute_end - f.end_position + 1;

            if (abs_start <= 0) {
                continue;
            }
            if (!matches.contains(f.pattern_id)) {
                matches[f.pattern_id].emplace_back(abs_start);
            }
            else {
                for (auto it = matches[f.pattern_id].end(); it != matches[f.pattern_id].begin();) {
                    --it;
                    int dist = (abs_start) - it->abs_start;
                    if (dist == 0) {
                        ++it->votes;
                        break;
                    }
                    // if (dist > 0 || it->votes == trie.getPatternFragments(f.pattern_id)) {
                    if (dist > 0) {
                        matches[f.pattern_id].emplace_back(abs_start);
                        break;
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

    std::vector<std::vector<std::pair<size_t, size_t>>> getResults(void) {
        std::vector<std::vector<std::pair<size_t, size_t>>> result;
        result.resize(trie.getPatternsNumber());
        for (auto& [pattern_id, v_m] : matches) {
            for (size_t i = 0; i < v_m.size(); ++i) {
                if (v_m[i].votes != trie.getPatternFragments(pattern_id)) {
                    std::swap(v_m[i], v_m.back());
                    v_m.pop_back();
                    --i;
                }
                else if (v_m[i].abs_start + trie.getPatternLength(pattern_id) - 1 <= absolute_pos) {
                    result[pattern_id - 1].push_back(std::move(getLineAndColumn(v_m[i].abs_start)));
                }
            }
        }
        return result;
    }

    size_t getAbsEndPos() const {
        return absolute_pos;
    }
};

class PatternMatcher {
    Trie trie;
    Scanner scanner;
    std::vector<size_t> jokers_id;

    void separatePatterns(const std::vector<Pattern>& patterns) {
        for (int i = 0; i < patterns.size(); ++i) {
            bool all_jokers = true;
            for (const auto& s : patterns[i].symbols) {
                if (!s.isJoker) {
                    all_jokers = false;
                    break;
                }
            }
            if (all_jokers) {
                jokers_id.push_back(i + 1);
            }
        }
    }

public:
    PatternMatcher(const std::vector<Pattern>& patterns) : trie(patterns), scanner(trie) {
        separatePatterns(patterns);
    }

    void feedLine(const std::string& line) {
        std::istringstream iss(std::move(line));
        unsigned num;
        while (iss >> num) {
            scanner.feedSymbol(num);
        }
        scanner.feedNewline();
    }

    std::vector<std::vector<std::pair<size_t, size_t>>> getResults() {
        std::vector<std::vector<std::pair<size_t, size_t>>> result(std::move(scanner.getResults()));
        for (const auto& j_id : jokers_id) {
            size_t len = trie.getPatternLength(j_id);
            size_t max_pos = scanner.getAbsEndPos() - len + 1;
            for (size_t start_pos = 1; start_pos <= max_pos; ++start_pos) {
                result[j_id - 1].push_back(std::move(scanner.getLineAndColumn(start_pos)));
            }
        }
        return result;
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

    PatternMatcher matcher(patterns);

    if (!buffer.empty()) {
        for (auto& line : buffer) {
            matcher.feedLine(line);
        }
        buffer.clear();
    }

    while (std::getline(std::cin, line)) {
        matcher.feedLine(line);
    }

    auto result = matcher.getResults();
    for (size_t i = 0; i < result.size(); ++i) {
        for (const auto& match : result[i]) {
            std::cout << match.first << ", " << match.second;
            if (patterns.size() > 1) {
                std::cout << ", " << i + 1;
            }
            std::cout << '\n';
        }
    }

    
}