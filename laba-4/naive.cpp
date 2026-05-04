// naive_multi.cpp — наивный поиск нескольких образцов с джокерами
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstddef>
#include <algorithm>

struct Symbol {
    unsigned int value;
    bool isJoker = false;
    Symbol(unsigned int v, bool j) : value(v), isJoker(j) {}
};

// Наивный поиск: возвращает 0-индексы начал вхождений
std::vector<size_t> naive_search(const std::vector<unsigned int>& text,
                                 const std::vector<Symbol>& pattern) {
    std::vector<size_t> occ;
    if (pattern.empty() || text.size() < pattern.size())
        return occ;
    const size_t n = text.size();
    const size_t m = pattern.size();
    for (size_t i = 0; i <= n - m; ++i) {
        bool match = true;
        for (size_t j = 0; j < m; ++j) {
            if (!pattern[j].isJoker && text[i + j] != pattern[j].value) {
                match = false;
                break;
            }
        }
        if (match) occ.push_back(i);
    }
    return occ;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // 1. Чтение образцов (каждая строка — один образец)
    //    Конец ввода образцов — пустая строка
    std::vector<std::vector<Symbol>> patterns;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) break; // пустая строка разделяет образцы и текст
        std::istringstream pat_stream(line);
        std::vector<Symbol> pattern;
        std::string token;
        while (pat_stream >> token) {
            if (token == "?") {
                pattern.emplace_back(0, true);
            } else {
                try {
                    unsigned int num = std::stoul(token);
                    pattern.emplace_back(num, false);
                } catch (...) {
                    std::cerr << "Invalid token in pattern: " << token << std::endl;
                    return 1;
                }
            }
        }
        if (!pattern.empty())
            patterns.push_back(std::move(pattern));
    }

    if (patterns.empty()) {
        return 0; // нет образцов
    }

    // 2. Чтение текста с запоминанием начал строк (для пересчёта в строки/столбцы)
    std::vector<unsigned int> text;
    std::vector<size_t> line_starts;
    line_starts.push_back(0); // начало первой строки

    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        unsigned int num;
        while (iss >> num) {
            text.push_back(num);
        }
        line_starts.push_back(text.size());
    }

    // 3. Для каждого образца находим вхождения и выводим результат
    for (size_t p = 0; p < patterns.size(); ++p) {
        auto occ = naive_search(text, patterns[p]);
        for (size_t pos : occ) {
            // Поиск строки, в которой находится позиция pos
            auto it = std::upper_bound(line_starts.begin(), line_starts.end(), pos);
            size_t line_idx = std::distance(line_starts.begin(), it) - 1;
            size_t line_start = line_starts[line_idx];
            size_t col = pos - line_start + 1; // столбец с 1
            // Вывод: "строка, столбец" и, если образцов >1, добавляем номер образца
            std::cout << (line_idx + 1) << ", " << col;
            if (patterns.size() > 1)
                std::cout << ", " << (p + 1);
            std::cout << '\n';
        }
    }

    return 0;
}