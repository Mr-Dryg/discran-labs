#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <vector>

class Scaner {
    std::string text;
    std::vector<size_t> suffix_array;
    std::map<std::pair<size_t, size_t>, size_t> lcp;

    void sortSuffixArray() {
        // TODO: radix
        std::sort(suffix_array.begin(), suffix_array.end(),
            [this](size_t a, size_t b) {
                return text.substr(a) < text.substr(b);
            }
        );
    }

    static size_t _calcLCP(
        std::string::iterator first_a, std::string::iterator last_a,
        std::string::iterator first_b, std::string::iterator last_b
    ) {
        size_t lcp = 0;
        while (first_a != last_a && first_b != last_b && *first_a == *first_b) {
            ++lcp;
            ++first_a;
            ++first_b;
        }
        return lcp;
    }

    size_t calcLCP_Pattern(size_t text_i, std::string& pattern) {
        return _calcLCP(
            text.begin() + text_i,
            text.end(),
            pattern.begin(),
            pattern.end()
        );
    }

    size_t calcLCP_Text(size_t text_i, size_t text_j) {
        return _calcLCP(
            text.begin() + text_i,
            text.end(),
            text.begin() + text_j,
            text.end()
        );
    }

    size_t buildLCP_Rec(size_t i, size_t j) {
        auto it = lcp.find({i, j});
        if (it != lcp.end()) {
            return it->second;
        }
        auto m = (i + j) / 2;
        size_t lcp_ij = std::min(buildLCP_Rec(i , m), buildLCP_Rec(m, j));
        lcp[{i, j}] = lcp_ij;
        return lcp_ij;
    };

    void buildLCP() {
        size_t max_j = text.length() - 1;
        for (size_t i = 0; i < max_j; ++i) {
            lcp[{i, i + 1}] = calcLCP_Text(
                suffix_array[i],
                suffix_array[i + 1]
            );
        }
        buildLCP_Rec(0, max_j);
    }

public:
    Scaner(std::string& str) : text(str) {
        for (size_t i = 0; i < text.length(); ++i) {
            suffix_array.push_back(i);
        }
        sortSuffixArray();
        buildLCP();
    }

    const std::vector<size_t>& getSuffixArray() {
        return suffix_array;
    }

    const std::map<std::pair<size_t, size_t>, size_t>& getLCP() {
        return lcp;
    }

    // TODO: find
};

int main() {
    std::string text;
    std::cin >> text;

    Scaner sc(text);

    auto sa = sc.getSuffixArray();
    std::cout << "SUFFIX ARRAY: [";
    for (const auto& elem : sa) {
        std::cout << elem << ", ";
    }
    std::cout << "]\n";

    auto lcp = sc.getLCP();
    std::cout << "LCP: {\n";
    for (const auto& [key, val] : lcp) {
        const auto& [i, j] = key;
        std::cout << "\t(" << i << "," << j << ") : " << val << ";\n";
    }
    std::cout << "}\n";
}
