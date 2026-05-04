#include <cstddef>
#include <iostream>
#include <sstream>
#include <vector>
#include <random>
#include <string>
#include <algorithm>
#include <set>

struct Symbol {
    unsigned int value;
    bool isJoker = false;
};

int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <text_length> <\"pattern\"> [occurrences] [max_val] [seed]\n";
        std::cerr << "Example: " << argv[0] << " 100000 \"1 ? 3\" 5 1000000 42\n";
        return 1;
    }

    size_t text_length = std::stoull(argv[1]);
    std::string pattern_str = argv[2];
    size_t occurrences = (argc > 3) ? std::stoull(argv[3]) : std::max(
        static_cast<size_t>(1),
        text_length / pattern_str.size() / 10
    );
    unsigned int max_val = (argc > 4) ? static_cast<unsigned int>(std::stoul(argv[4])) : 100;
    unsigned int seed = (argc > 5) ? static_cast<unsigned int>(std::stoul(argv[5])) : std::random_device{}();

    std::istringstream pat_stream(pattern_str);
    std::string token;
    std::vector<Symbol> pattern;
    while (pat_stream >> token) {
        if (token == "?") {
            pattern.push_back({0, true});
        } else {
            try {
                unsigned int num = std::stoul(token);
                pattern.push_back({num, false});
            } catch (...) {
                std::cerr << "Invalid number in pattern: " << token << std::endl;
                return 1;
            }
        }
    }

    if (pattern.empty()) {
        std::cerr << "Pattern must contain at least one symbol\n";
        return 1;
    }

    size_t pat_len = pattern.size();
    if (pat_len > text_length) {
        std::cerr << "Pattern length (" << pat_len << ") exceeds text length (" << text_length << ")\n";
        return 1;
    }

    std::mt19937 rng(seed);
    std::uniform_int_distribution<unsigned int> val_dist(0, max_val);
    std::vector<unsigned int> text(text_length);
    for (auto& x : text) x = val_dist(rng);

    if (occurrences > 0) {
        size_t max_start = text_length - pat_len;
        if (max_start == 0) {
            occurrences = 1;
        } else {
            std::vector<size_t> positions(max_start + 1);
            std::iota(positions.begin(), positions.end(), 0);
            std::shuffle(positions.begin(), positions.end(), rng);
            std::vector<size_t> selected;
            std::set<size_t> used;
            for (size_t pos : positions) {
                bool ok = true;
                auto it = used.lower_bound(pos);
                if (it != used.end() && *it < pos + pat_len) ok = false;
                if (it != used.begin()) {
                    --it;
                    if (*it + pat_len > pos) ok = false;
                }
                if (ok) {
                    selected.push_back(pos);
                    used.insert(pos);
                    if (selected.size() == occurrences) break;
                }
            }
            occurrences = selected.size();
            for (size_t start : selected) {
                for (size_t j = 0; j < pat_len; ++j) {
                    if (!pattern[j].isJoker) {
                        text[start + j] = pattern[j].value;
                    }
                }
            }
        }
    }

    std::cout << pattern_str << '\n';

    const size_t per_line = 20;
    for (size_t i = 0; i < text.size(); ++i) {
        std::cout << text[i];
        if ((i + 1) % per_line == 0 || i + 1 == text.size())
            std::cout << '\n';
        else
            std::cout << ' ';
    }

    return 0;
}