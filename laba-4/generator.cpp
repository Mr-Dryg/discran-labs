// gen_multi.cpp — генератор тестов для поиска нескольких образцов с джокерами
#include <iostream>
#include <sstream>
#include <vector>
#include <random>
#include <string>
#include <algorithm>
#include <set>
#include <getopt.h>   // для разбора аргументов, если нужно, но здесь простой позиционный разбор

struct Symbol {
    unsigned int value;
    bool isJoker = false;
};

int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // Использование: ./gen_multi <text_length> <occurrences_per_pattern> [max_val] [seed]
    // Образцы читаются со stdin (каждая строка – один образец)
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <text_length> <occurrences_per_pattern> [max_val] [seed]\n";
        std::cerr << "Patterns are read from stdin, one per line.\n";
        return 1;
    }

    size_t text_length = std::stoull(argv[1]);
    size_t occurrences_per_pattern = std::stoull(argv[2]);
    unsigned int max_val = (argc > 3) ? static_cast<unsigned int>(std::stoul(argv[3])) : 100;
    unsigned int seed = (argc > 4) ? static_cast<unsigned int>(std::stoul(argv[4])) : 42;

    // Чтение образцов со stdin
    std::vector<std::vector<Symbol>> patterns;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue; // игнорируем пустые строки в режиме чтения образцов
        std::istringstream pat_stream(line);
        std::vector<Symbol> pattern;
        std::string token;
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
        if (!pattern.empty())
            patterns.push_back(std::move(pattern));
    }

    if (patterns.empty()) {
        std::cerr << "No patterns provided.\n";
        return 1;
    }

    // Проверка, что суммарная длина всех вставок не превышает text_length
    size_t total_insert_len = 0;
    for (const auto& pat : patterns)
        total_insert_len += pat.size() * occurrences_per_pattern;
    if (total_insert_len > text_length) {
        std::cerr << "Warning: total length of all pattern insertions (" 
                  << total_insert_len << ") exceeds text length (" << text_length 
                  << "). Some insertions will be skipped.\n";
    }

    // Генерация случайного текста
    std::mt19937 rng(seed);
    std::uniform_int_distribution<unsigned int> val_dist(0, max_val);
    std::vector<unsigned int> text(text_length);
    for (auto& x : text) x = val_dist(rng);

    // Набор уже занятых интервалов для предотвращения пересечений
    std::set<std::pair<size_t, size_t>> occupied; // [start, end) интервалы
    auto is_free = [&](size_t start, size_t length) -> bool {
        if (start + length > text_length) return false;
        auto it = occupied.lower_bound({start, 0});
        if (it != occupied.end() && it->first < start + length)
            return false;
        if (it != occupied.begin()) {
            --it;
            if (it->second > start)
                return false;
        }
        return true;
    };

    // Вставка каждого образца occurrences_per_pattern раз
    std::uniform_int_distribution<size_t> start_dist(0, text_length - 1);
    size_t inserted = 0;
    for (const auto& pattern : patterns) {
        for (size_t rep = 0; rep < occurrences_per_pattern; ++rep) {
            // Попытки найти свободный участок (не слишком агрессивно)
            bool placed = false;
            for (int attempts = 0; attempts < 1000; ++attempts) {
                size_t start = start_dist(rng);
                if (start > text_length - pattern.size()) continue;
                if (is_free(start, pattern.size())) {
                    for (size_t j = 0; j < pattern.size(); ++j) {
                        if (!pattern[j].isJoker) {
                            text[start + j] = pattern[j].value;
                        }
                    }
                    occupied.insert({start, start + pattern.size()});
                    placed = true;
                    ++inserted;
                    break;
                }
            }
            if (!placed) {
                std::cerr << "Could not place pattern after 1000 attempts.\n";
            }
        }
    }

    // Вывод образцов (по одному на строку), затем пустая строка, затем текст
    for (const auto& pat : patterns) {
        for (size_t i = 0; i < pat.size(); ++i) {
            if (i > 0) std::cout << ' ';
            if (pat[i].isJoker) std::cout << '?';
            else std::cout << pat[i].value;
        }
        std::cout << '\n';
    }
    std::cout << '\n'; // пустая строка-разделитель

    // Вывод текста (по 20 чисел в строке)
    const size_t per_line = 20;
    for (size_t i = 0; i < text.size(); ++i) {
        std::cout << text[i];
        if ((i + 1) % per_line == 0 || i + 1 == text.size())
            std::cout << '\n';
        else
            std::cout << ' ';
    }

    std::cerr << "Inserted " << inserted << " occurrences of patterns.\n";
    return 0;
}