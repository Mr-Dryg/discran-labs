#include "src.h"
#include <algorithm>

int main(void) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    auto func = [](std::vector<std::pair<Date, std::string>>& data) {
        auto comp = [](const auto& a, const auto& b) {
            return a.first.sort_key < b.first.sort_key;
        };
        std::stable_sort(data.begin(), data.end(), comp);
    };

    read_sort_write(func);

    return 0;
}