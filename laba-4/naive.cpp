#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstddef>
#include <algorithm>
#include <chrono>

struct Symbol {
    unsigned int value;
    bool isJoker = false;
    Symbol(unsigned int v, bool j) : value(v), isJoker(j) {}
};

std::vector<size_t> naive_search(const std::vector<unsigned int>& text,
                                 const std::vector<Symbol>& pattern) {
    std::vector<size_t> occ;
    if (pattern.empty() || text.size() < pattern.size())
        return occ;
    const size_t n = text.size();
    const size_t m = pattern.size();
    for (size_t i = 0; i <= n - m; ++i) {
        bool match = true;
        for (size_t j = 0; j < m; ++j) {
            if (!pattern[j].isJoker && text[i + j] != pattern[j].value) {
                match = false;
                break;
            }
        }
        if (match) occ.push_back(i);
    }
    return occ;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    auto start_build = std::chrono::high_resolution_clock::now();

    std::string pattern_line;
    if (!std::getline(std::cin, pattern_line)) {
        return 0;
    }

    std::vector<Symbol> pattern;
    std::istringstream pat_stream(pattern_line);
    std::string token;
    while (pat_stream >> token) {
        if (token == "?") {
            pattern.emplace_back(0, true);
        } else {
            try {
                unsigned int num = std::stoul(token);
                pattern.emplace_back(num, false);
            } catch (...) {
                std::cerr << "Invalid token in pattern: " << token << std::endl;
                return 1;
            }
        }
    }

    std::vector<unsigned int> text;
    std::vector<size_t> line_starts;
    line_starts.push_back(0);

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        unsigned int num;
        while (iss >> num) {
            text.push_back(num);
        }
        line_starts.push_back(text.size());
    }

    auto occ = naive_search(text, pattern);

    for (size_t pos : occ) {
        auto it = std::upper_bound(line_starts.begin(), line_starts.end(), pos);
        size_t line_idx = std::distance(line_starts.begin(), it) - 1;
        size_t line_start = line_starts[line_idx];
        size_t col = pos - line_start + 1; // столбец с 1
        std::cout << (line_idx + 1) << ", " << col << '\n';
    }

    return 0;
}