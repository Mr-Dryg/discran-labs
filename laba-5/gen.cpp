#include <random>
#include <string>
#include <iostream>

int main(int argc, char* argv[]) {
    int n = 100'000;
    
    if (argc > 1) {
        n = std::stoi(argv[1]);
    }

    std::string text(n, 'a');
    std::cout << text << "\n";

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> len_dist(2, 100);
    std::uniform_int_distribution<int> char_dist(0, 25);

    int pattern_count = 200;
    for (int i = 0; i < pattern_count; ++i) {
        int len = len_dist(rng);
        std::string pattern;
        for (int j = 0; j < len; ++j) {
            pattern += 'a' + char_dist(rng);
        }
        std::cout << pattern << "\n";
    }

    return 0;
}