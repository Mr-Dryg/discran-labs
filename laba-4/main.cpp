#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <queue>
#include <stdexcept>
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

    Pattern() : symbols() {}
    Pattern(const std::vector<Symbol>& p) : symbols(p) {}

    friend std::istream& operator>>(std::istream& is, Pattern& pattern) {
        unsigned int num;
        std::string token;

        std::string line;
        std::getline(is, line);
        std::istringstream iss(line);
        
        while (iss >> token) {
            if (token == "?") {
                pattern.symbols.emplace_back(0, true);
            } else {
                try {
                    num = std::stoul(token);
                    pattern.symbols.emplace_back(num, false);
                } catch (...) {
                    throw std::runtime_error("Invalid token: \"" + token + "\"");
                }
            }
        }
        return is;
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

struct Match {
    size_t line;
    size_t pos;

    Match(size_t line, size_t pos) : line(line), pos(pos) {}
};

class AhoCorasick {
    struct Node {
        std::unordered_map<unsigned int, Node*> next;
        std::vector<size_t> fragment_offsets;
        Node* fail = nullptr;
        Node* out = nullptr;

        bool isEnd() const {
            return !fragment_offsets.empty();
        }

        ~Node() {
            for (auto& [_, child] : next) {
                delete child;
            }
        }
    };

    Node* root;
    const Node* cur;
    std::vector<size_t> row_match_starts;
    size_t cur_pos = 0;
    std::vector<size_t> line_starts;
    size_t fragment_number = 0;
    size_t pattern_length = 0;

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

    void addFragment(const std::vector<unsigned int>& fragment, size_t i_end) {
        Node* cur_node = root;
        for (const auto& symbol : fragment) {
            if (!cur_node->next.contains(symbol)) {
                cur_node->next[symbol] = new Node;
            }
            cur_node = cur_node->next[symbol];
        }
        cur_node->fragment_offsets.emplace_back(i_end);
        ++fragment_number;
    }

    void saveMatch(size_t abs_end, const Node* node) {
        for (const auto& offset : node->fragment_offsets) {
            if (abs_end < offset) {
                continue;
            }
            row_match_starts.emplace_back(abs_end - offset);
        }
    }

    Match getLineAndColumn(size_t start_pos) {
        auto it = std::upper_bound(line_starts.begin(), line_starts.end(), start_pos);
        size_t line = std::distance(line_starts.begin(), it);
        size_t line_start = *(--it);
        return {line, start_pos - line_start + 1};
    }

public:
    AhoCorasick(const std::vector<Symbol>& pattern) : AhoCorasick(Pattern(pattern)) {}

    AhoCorasick(const Pattern& pattern) {
        cur = root = new Node;
        pattern_length = pattern.symbols.size();
        size_t end_position = -1;
        std::vector<unsigned int> fragment;

        for (const auto& symbol : pattern.symbols) {
            if (symbol.isJoker) {
                if (!fragment.empty()) {
                    addFragment(fragment, end_position);
                    fragment.clear();
                }
            }
            else {
                fragment.push_back(symbol.value);
            }
            ++end_position;
        }
        if (!fragment.empty()) {
            addFragment(fragment, end_position);
        }
        buildLinks();
        line_starts.push_back(0);
    }

    ~AhoCorasick() {
        delete root;
    }

    void feedSymbol(unsigned int symbol) {
        while (!cur->next.contains(symbol) && cur != root) {
            cur = cur->fail;
        }
        if (cur->next.contains(symbol)) {
            cur = cur->next.find(symbol)->second;
        }
        
        if (cur->isEnd()) {
            saveMatch(cur_pos, cur);
        }

        Node* out = cur->out;
        while (out) {
            saveMatch(cur_pos, out);
            out = out->out;
        }
        ++cur_pos;
    }

    void feedNewline() {
        line_starts.push_back(cur_pos);
    }

    std::vector<Match> getResults() {
        std::vector<int> votes(cur_pos, 0);
        for (auto& abs_start : row_match_starts) {
            if (abs_start + pattern_length <= cur_pos) {
                ++votes[abs_start];
            }
        }

        std::vector<Match> matches;
        for (size_t i = 0; i < votes.size(); ++i) {
            if (votes[i] == fragment_number && i + pattern_length <= cur_pos) {
                matches.push_back(getLineAndColumn(i));
            }
        }
        return matches;
    }
};

// int main() {
//     std::ios::sync_with_stdio(false);
//     std::cin.tie(nullptr);

//     Pattern pattern;

//     std::cin >> pattern;

//     AhoCorasick ac(pattern);

//     char c;
//     unsigned int value = 0;
//     enum Status {
//         waiting,
//         reading_number
//     };
//     Status status = waiting;

//     while ((c = getchar()) && c != EOF) {
//         switch (status) {
//         case waiting:
//             if (std::isdigit(c)) {
//                 value = c - '0';
//                 status = reading_number;
//             }
//             break;
//         case reading_number:
//             if (std::isdigit(c)) {
//                 value = 10 * value + c - '0';
//             }
//             else {
//                 ac.feedSymbol(value);
//                 status = waiting;
//             }
//             break;
//         }
//         if (c == '\n') {
//             ac.feedNewline();
//         }
//     }

//     auto res = ac.getResults();
//     for (auto& match : res) {
//         std::cout << match.line << ", " << match.pos << '\n';
//     }
// }