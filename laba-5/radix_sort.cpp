#include <algorithm>
#include <cstddef>
#include <chrono>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

class Scaner {
    char max_key;
    std::vector<int> cnt;
    std::string text;
    std::vector<size_t> suffix_array;

    void countingSort(size_t shift) {
        std::fill(cnt.begin(), cnt.end(), 0);
        char c;
        
        for (auto& suf_start : suffix_array) {
            if (suf_start + shift < text.length()) {
                c = text[suf_start + shift];
            } else {
                c = 0;
            }
            ++cnt[c];
        }
        
        for (size_t i = 1; i < cnt.size(); ++i) {
            cnt[i] += cnt[i - 1];
        }
        
        std::vector<size_t> new_sa(suffix_array.size());
        
        for (long i = static_cast<long>(suffix_array.size()) - 1; i >= 0; --i) {
            if (suffix_array[i] + shift < text.length()) {
                c = text[suffix_array[i] + shift];
            } else {
                c = 0;
            }
            new_sa[--cnt[c]] = suffix_array[i];
        }
        
        suffix_array = std::move(new_sa);
    }

    void radixSortSuffixArray() {
        for (long i = static_cast<long>(text.length()) - 1; i >= 0; --i) {
            countingSort(i);
        }
    }

    void sortSuffixArray() {
        // std::sort(suffix_array.begin(), suffix_array.end(),
        //     [this](size_t a, size_t b) {
        //         return text.compare(a, std::string::npos,
        //                           text, b, std::string::npos) < 0;
        //     }
        // );

        radixSortSuffixArray();
    }

    int comparePatternWithSuffix(const std::string& pattern, size_t suffix_pos) {
        size_t i = 0;
        while (i < pattern.length() && suffix_pos + i < text.length()) {
            if (pattern[i] != text[suffix_pos + i]) {
                return pattern[i] < text[suffix_pos + i] ? -1 : 1;
            }
            ++i;
        }
        if (i == pattern.length()) {
            return 0;
        }
        else if (suffix_pos + i == text.length()) {
            return 1;
        }
        else {
            return -1;
        }
    }

    size_t lowerBound(const std::string& pattern) {
        size_t lo = 0, hi = suffix_array.size();
        while (lo < hi) {
            size_t mid = (lo + hi) / 2;
            int cmp = comparePatternWithSuffix(pattern, suffix_array[mid]);
            if (cmp > 0) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo;
    }

    size_t upperBound(const std::string& pattern) {
        size_t lo = 0, hi = suffix_array.size();
        while (lo < hi) {
            size_t mid = (lo + hi) / 2;
            int cmp = comparePatternWithSuffix(pattern, suffix_array[mid]);
            if (cmp >= 0) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo;
    }

public:
    Scaner(const std::string& str) : text(str) {
        max_key = text[0];
        suffix_array.reserve(text.size());
        for (size_t i = 0; i < text.size(); ++i) {
            suffix_array.push_back(i);
            max_key = std::max(max_key, text[i]);
        }
        cnt.resize(max_key + 1, 0);
        sortSuffixArray();
    }

    const std::vector<size_t>& getSuffixArray() const {
        return suffix_array;
    }

    std::vector<size_t> find(const std::string& pattern) {
        std::vector<size_t> matches;
        if (pattern.empty() || text.empty()) return matches;
        
        size_t lb = lowerBound(pattern);
        size_t ub = upperBound(pattern);

        for (size_t i = lb; i < ub; ++i) {
            matches.push_back(suffix_array[i]);
        }
        std::sort(matches.begin(), matches.end());
        return matches;
    }
};

int main() {
    std::string str;
    std::getline(std::cin, str);
    
    auto build_start = std::chrono::high_resolution_clock::now();
    Scaner sc(str);
    auto build_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> build_elapsed = build_end - build_start;

    double total_search_time = 0.0;
    size_t pattern_id = 0;
    str.clear();
    while (std::getline(std::cin, str)) {
        if (str.empty()) continue;

        auto search_start = std::chrono::high_resolution_clock::now();
        auto matches = sc.find(str);
        auto search_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> search_elapsed = search_end - search_start;
        total_search_time += search_elapsed.count();

        ++pattern_id;
        if (matches.empty()) continue;
        std::cout << pattern_id << ": ";
        for (auto it = matches.begin(); it != matches.end(); ++it) {
            std::cout << *it + 1;
            if (std::next(it) != matches.end()) {
                std::cout << ", ";
            }
        }
        std::cout << "\n";
    }

    std::cout << std::fixed;
    std::cout << "Suffix array build time: " << build_elapsed.count() << " sec\n";
    std::cout << "Suffix array search time: " << total_search_time << " sec\n";
}