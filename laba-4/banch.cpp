#include "main.cpp"
#include <cstddef>
#include <chrono>
#include <iterator>
#include <random>
#include <vector>

unsigned int rand_int(unsigned int min, unsigned int max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<unsigned int> dist(min, max);
    return dist(rng);
}

enum Scenario { AHO_WINS, AHO_LOSES };

// ------------------------------------------------------------
// Сценарий 1: АХО ВЫИГРЫВАЕТ
// Паттерн:  ? ? ? ? 4 2 3 9 1
// Текст: случайные числа 0..99, твёрдый суффикс 4 2 3 9 1 вставлен редко
// ------------------------------------------------------------
std::vector<unsigned int> generate_text_scenario1(size_t size) {
    std::vector<unsigned int> text;
    text.reserve(size);
    
    const std::vector<unsigned int> suffix = {4, 2, 3, 9, 1};
    const size_t period = 10000;
    
    size_t next_insert = period;
    
    for (size_t i = 0; i < size; ) {
        if (i == next_insert && i + suffix.size() <= size) {
            text.insert(text.end(), {0, 0, 0, 0, 4, 2, 3, 9, 1});
            i += 9;
            next_insert += period;
        } else {
            unsigned int val = rand_int(0, 99);
            // if (val == 4) val = 99;
            text.push_back(val);
            ++i;
        }
    }
    
    text.resize(size);
    return text;
}

std::vector<Symbol> generate_pattern_scenario1() {
    return {
        Symbol(0, true),   // ?
        Symbol(0, true),   // ?
        Symbol(0, true),   // ?
        Symbol(0, true),   // ?
        Symbol(4, false),
        Symbol(2, false),
        Symbol(3, false),
        Symbol(9, false),
        Symbol(1, false)
    };
}

// ------------------------------------------------------------
// Сценарий 2: АХО ПРОИГРЫВАЕТ
// Паттерн:  1 ? 1 ? 1
// Текст: 90% единиц, 10% мусора (чтобы не 100% совпадений)
// ------------------------------------------------------------
std::vector<unsigned int> generate_text_scenario2(size_t size) {
    std::vector<unsigned int> text;
    text.reserve(size);
    
    for (size_t i = 0; i < size; ++i) {
        if (rand_int(1, 100) <= 90) {
            text.push_back(1);
        } else {
            text.push_back(2);
        }
    }
    
    return text;
}

std::vector<Symbol> generate_pattern_scenario2() {
    return {
        Symbol(1, false),
        Symbol(0, true),   // ?
        Symbol(1, false),
        Symbol(0, true),   // ?
        Symbol(1, false)
    };
}

std::vector<std::vector<unsigned int>> split_text(std::vector<unsigned int>& text) {
    const size_t max_line_length = 50;
    const size_t min_line_length = max_line_length / 10 * 6;

    std::vector<std::vector<unsigned int>> res;
    res.reserve(text.size() / min_line_length + 1);

    auto it = text.begin();
    while (std::distance(it, text.end()) > 0) {
        size_t next_line = rand_int(min_line_length, max_line_length);
        res.emplace_back(it, it + next_line);
        it += next_line;
    }
    return res;
}

std::vector<std::vector<unsigned int>> generate_text(size_t size, Scenario type) {
    std::vector<unsigned int> text;
    switch (type) {
        case Scenario::AHO_WINS:
            text = generate_text_scenario1(size);
            break;
        case Scenario::AHO_LOSES:
            text = generate_text_scenario2(size);
            break;
    }
    return split_text(text);
}

Pattern generate_pattern(Scenario type) {
    switch (type) {
        case Scenario::AHO_WINS:   return generate_pattern_scenario1();
        case Scenario::AHO_LOSES:  return generate_pattern_scenario2();
    }
    return {};
}

template<typename Func>
double measure(Func f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

std::vector<Match> naive_search(const std::vector<std::vector<unsigned int>>& text, const Pattern& pattern) {
    std::vector<unsigned int> text_in_line;
    std::vector<size_t> line_starts;
    line_starts.push_back(0);

    for (const auto& line : text) {
        for (const auto& num : line) {
            text_in_line.push_back(num);
        }
        line_starts.push_back(text_in_line.size());
    }

    std::vector<size_t> occ;
    if (pattern.symbols.empty() || text_in_line.size() < pattern.symbols.size())
        return {};
    const size_t n = text_in_line.size();
    const size_t m = pattern.symbols.size();
    for (size_t i = 0; i <= n - m; ++i) {
        bool match = true;
        for (size_t j = 0; j < m; ++j) {
            if (!pattern.symbols[j].isJoker && text_in_line[i + j] != pattern.symbols[j].value) {
                match = false;
                break;
            }
        }
        if (match) occ.push_back(i);
    }

    std::vector<Match> matches;
    matches.reserve(occ.size());

    for (size_t pos : occ) {
        auto it = std::upper_bound(line_starts.begin(), line_starts.end(), pos);
        size_t line_idx = std::distance(line_starts.begin(), it) - 1;
        size_t line_start = line_starts[line_idx];
        size_t col = pos - line_start + 1;
        matches.emplace_back(line_idx + 1, col);
    }
    return matches;
}

std::vector<Match> aho_corasick_search(const std::vector<std::vector<unsigned int>>& text, const Pattern& pattern) {
    AhoCorasick ac(pattern);

    for (const auto& line : text) {
        for (const auto& num : line) {
            ac.feedSymbol(num);
        }
        ac.feedNewline();
    }

    return ac.getResults();
}

#include <iomanip>
#include <iostream>

int main() {
    for (auto type : {Scenario::AHO_WINS, Scenario::AHO_LOSES})
    {
        auto pattern = generate_pattern(type);
        std::cout << "pattern:" << pattern << "\n";
        std::cout << std::left
                  << std::setw(12) << "n"
                  << std::setw(14) << "Naive (ms)"
                  << std::setw(14) << "Aho (ms)"
                  << std::setw(12) << "Speedup"
                  << "\n";
        std::cout << std::string(50, '-') << "\n";
        for (size_t size : {10'000, 100'000, 1'000'000}) {
            auto text = generate_text(size, type);

            double t_naive = measure([&]() { naive_search(text, pattern); });
            double t_aho   = measure([&]() { aho_corasick_search(text, pattern); });

            std::cout << std::left
                      << std::setw(12) << size
                      << std::setw(14) << std::fixed << std::setprecision(2) << t_naive
                      << std::setw(14) << std::fixed << std::setprecision(2) << t_aho
                      << std::setw(12) << std::fixed << std::setprecision(2) << (t_naive / t_aho)
                      << "\n";
        }
        std::cout << "\n";
    }
}