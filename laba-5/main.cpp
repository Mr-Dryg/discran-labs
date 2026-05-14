#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <map>
#include <utility>
#include <vector>

class Scaner {
    std::string text;
    std::set<size_t> matches;
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
        if (i > j) return 0;
        auto it = lcp.find({i, j});
        if (it != lcp.end()) {
            return it->second;
        }
        auto m = (i + j) / 2;
        size_t lcp_ij = std::min(buildLCP_Rec(i , m), buildLCP_Rec(m + 1, j));
        lcp[{i, j}] = lcp_ij;
        return lcp_ij;
    };

    void buildLCP() {
        size_t max_j = text.length() - 1;
        for (size_t i = 0; i < max_j; ++i) {
            lcp[{i, i}] = text.length() - i;
            lcp[{i, i + 1}] = calcLCP_Text(
                suffix_array[i],
                suffix_array[i + 1]
            );
        }
        lcp[{max_j, max_j}] = text.length() - max_j;
        buildLCP_Rec(0, max_j);
    }

    int cmp_pattern_with_suffix(const std::string& pattern, size_t suffix_pos, size_t i = 0) {
        while (i < pattern.length() && suffix_pos + i < text.length()) {
            if (pattern[i] != text[suffix_pos + i]) {
                return pattern[i] < text[suffix_pos + i] ? -1 : 1;
            }
            ++i;
        }
        if (i == pattern.length() && suffix_pos + i == text.length()) {
            return 0;
        }
        if (i == pattern.length()) {
            return -1;
        }
        return 1;
    }

    void cmp_with_middle(
        std::string& pattern, size_t L, size_t R,
        size_t lcp_l, size_t lcp_r,
        size_t M, size_t lcp_m, size_t i
    ) {
        size_t i_eot = text.length() - suffix_array[M];

        int cmp = cmp_pattern_with_suffix(pattern, suffix_array[M], i);
        if (cmp == 0) {
            matches.insert(suffix_array[M]);

            find(pattern, L, M, lcp_l, lcp_m);
            lcp_m = calcLCP_Pattern(suffix_array[M + 1], pattern);
            find(pattern, M + 1, R, lcp_m, lcp_r);
        }
        else if (cmp < 0) {
            find(pattern, L, M, lcp_l, lcp_m);
        }
        else {
            lcp_m = calcLCP_Pattern(suffix_array[M + 1], pattern);
            find(pattern, M + 1, R, lcp_m, lcp_r);
        }
    }

    void find(
        std::string& pattern, size_t L, size_t R,
        size_t lcp_l, size_t lcp_r
    ) {
        if (L > R) return;

        switch (R - L) {
        case 1:
            if (cmp_pattern_with_suffix(pattern, suffix_array[R]) == 0) {
                matches.insert(suffix_array[R]);
            }
        case 0:
            if (cmp_pattern_with_suffix(pattern, suffix_array[L]) == 0) {
                matches.insert(suffix_array[L]);
            }
            return;
        default:
            break;
        }

        size_t M = (L + R) / 2;
        size_t lcp_m = calcLCP_Pattern(suffix_array[M], pattern);
        
        if (lcp_l == lcp_r) {
            cmp_with_middle(pattern, L, R, lcp_l, lcp_r, M, lcp_m, lcp_l);
        }
        else if (lcp_l > lcp_r) {
            long long cmp = static_cast<long long>(lcp[{suffix_array[L], suffix_array[M]}]) - static_cast<long long>(lcp_l);

            if (cmp == 0) {
                cmp_with_middle(pattern, L, R, lcp_l, lcp_r, M, lcp_m, lcp_l);
            }
            else if (cmp > 0) {
                lcp_m = calcLCP_Pattern(suffix_array[M + 1], pattern);
                find(pattern, M + 1, R, lcp_m, lcp_r);
            }
            else {
                find(pattern, L, M, lcp_l, lcp_m);
            }
        }
        else if (lcp_l < lcp_r) {
            long long cmp = static_cast<long long>(lcp[{suffix_array[M], suffix_array[R]}]) - static_cast<long long>(lcp_r);
            
            if (cmp == 0) {
                cmp_with_middle(pattern, L, R, lcp_l, lcp_r, M, lcp_m, lcp_r);
            }
            else if (cmp > 0) {
                find(pattern, L, M, lcp_l, lcp_m);
            }
            else {
                lcp_m = calcLCP_Pattern(suffix_array[M + 1], pattern);
                find(pattern, M + 1, R, lcp_m, lcp_r);
            }
        }
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

    std::set<size_t> find(std::string& pattern) {
        if (pattern.empty()) return {};
        matches.clear();
        size_t L = 0;
        size_t R = suffix_array.size() - 1;
        size_t lcp_l = calcLCP_Pattern(suffix_array[L], pattern);
        size_t lcp_r = calcLCP_Pattern(suffix_array[R], pattern);
        find(pattern, L, R, lcp_l, lcp_r);
        return matches;
    }
};

int main() {
    std::string str;
    std::cin >> str;

    Scaner sc(str);

    // auto sa = sc.getSuffixArray();
    // std::cout << "SUFFIX ARRAY: [";
    // for (const auto& elem : sa) {
    //     std::cout << elem << ", ";
    // }
    // std::cout << "]\n";

    // auto lcp = sc.getLCP();
    // std::cout << "LCP: {\n";
    // for (const auto& [key, val] : lcp) {
    //     const auto& [i, j] = key;
    //     std::cout << "\t(" << i << "," << j << ") : " << val << ";\n";
    // }
    // std::cout << "}\n";

    size_t pattern_id = 0;
    while (std::cin >> str) {
        std::set<size_t> matches = sc.find(str);
        // std::cout << "METKA 1\n";
        std::cout << ++pattern_id << ": ";
        // std::cout << "METKA 2\n";
        for (auto it = matches.begin(); it != matches.end(); ++it) {
            // std::cout << "METKA 3\n";
            std::cout << *it + 1;
            if (std::next(it) != matches.end()) {
                std::cout << ", ";
            }
            // std::cout << "METKA 4\n";
        }
        std::cout << "\n";
    }
}
