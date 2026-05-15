#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

class Scaner {
    std::string text;
    std::vector<size_t> suffix_array;

    void sortSuffixArray() {
        std::sort(suffix_array.begin(), suffix_array.end(),
            [this](size_t a, size_t b) {
                return text.compare(a, std::string::npos,
                                  text, b, std::string::npos) < 0;
            }
        );
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
        suffix_array.reserve(text.size());
        for (size_t i = 0; i < text.size(); ++i) {
            suffix_array.push_back(i);
        }
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
    
    Scaner sc(str);

    size_t pattern_id = 0;
    str.clear();
    while (std::getline(std::cin, str)) {
        if (str.empty()) continue;
        auto matches = sc.find(str);
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
}