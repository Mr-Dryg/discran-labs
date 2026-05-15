#include <iostream>
#include <string>
#include <vector>
#include <chrono>

class KMP {
    static std::vector<size_t> buildPrefix(const std::string& pattern) {
        std::vector<size_t> prefix(pattern.size(), 0);
        size_t k = 0;

        for (size_t q = 1; q < pattern.size(); ++q) {
            while (k > 0 && pattern[k] != pattern[q]) {
                k = prefix[k - 1];
            }
            if (pattern[k] == pattern[q]) {
                ++k;
            }
            prefix[q] = k;
        }

        return prefix;
    }
public:
    static std::vector<size_t> search(const std::string& text, const std::string& pattern) {
        std::vector<size_t> matches;
        if (pattern.empty() || text.empty()) return matches;

        std::vector<size_t> prefix = buildPrefix(pattern);
        size_t q = 0;

        for (size_t i = 0; i < text.size(); ++i) {
            while (q > 0 && pattern[q] != text[i]) {
                q = prefix[q - 1];
            }
            if (pattern[q] == text[i]) {
                ++q;
            }
            if (q == pattern.size()) {
                matches.push_back(i - pattern.size() + 1);
                q = prefix[q - 1];
            }
        }

        return matches;
    }
};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string text;
    std::getline(std::cin, text);

    std::string pattern;
    double total_search_time = 0.0;

    while (std::getline(std::cin, pattern)) {
        if (pattern.empty()) continue;

        auto start = std::chrono::high_resolution_clock::now();
        auto matches = KMP::search(text, pattern);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;
        total_search_time += elapsed.count();
    }

    std::cout << "KMP search time: " << std::fixed << total_search_time << " sec\n";

    return 0;
}