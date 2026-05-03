#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <utility>
#include <iomanip>
#include <queue>
#include <string>
#include <unordered_map>

// ========== Оригинальные классы ==========
struct Symbol {
    unsigned int value;
    bool isJoker = false;

    Symbol(unsigned int value, bool isJoker) : value(value), isJoker(isJoker) {}
};

struct Pattern {
    std::vector<Symbol> symbols;

    Pattern(std::istream& is) {
        unsigned int num;
        std::string token;
        
        while (is >> token) {
            if (token == "?") {
                symbols.emplace_back(0, true);
            } else {
                try {
                    num = std::stoul(token);
                    symbols.emplace_back(num, false);
                } catch (...) {
                    throw std::runtime_error("Invalid token: \"" + token + "\"");
                }
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& os, Pattern& pattern) {
        for (auto& symbol : pattern.symbols) {
            if (symbol.isJoker) {
                os << " ?";
            }
            else {
                os << " " << symbol.value;
            }
        }
        return os;
    }
};

struct FragmentInfo {
    size_t pattern_id;
    size_t end_position;
    size_t length;

    FragmentInfo(size_t p_id, size_t end_pos, size_t len) : pattern_id(p_id), end_position(end_pos), length(len) {}
};

struct Node {
    std::unordered_map<unsigned int, Node*> next;  // try map to dicrease time/memmory
    std::vector<FragmentInfo> fragments;
    Node* fail = nullptr;
    Node* out = nullptr;

    bool isEnd() const {
        return !fragments.empty();
    }

    ~Node() {
        for (auto& [_, child] : next) {
            delete child;
        }
    }
};

class Trie {
    Node* root;
    std::vector<size_t> pattern_fragments;
    std::vector<size_t> pattern_lengths;

    void buildLinks(void) {
        std::queue<Node*> nodes;
        nodes.push(root);

        root->fail = root;
        while (!nodes.empty()) {
            Node* parent = nodes.front();
            nodes.pop();

            for (const auto& [symbol, node] : parent->next) {
                nodes.push(node);

                if (parent == root) {
                    node->fail = root;
                }
                else {
                    Node* fail = parent->fail;
                    while (!fail->next.contains(symbol) && fail != root) {
                        fail = fail->fail;
                    }

                    if (fail->next.contains(symbol)) {
                        node->fail = fail->next[symbol];
                    }
                    else {
                        node->fail = fail;
                    }
                }

                if (node->fail->isEnd()) {
                    node->out = node->fail;
                }
                else if (node->fail->out) {
                    node->out = node->fail->out;
                }
            }
        }
    }

    void addFragment(const std::vector<unsigned int>& fragment, size_t p_id, size_t end_pos) {
        ++pattern_fragments[p_id - 1];
        Node* cur_node = root;
        for (const auto& symbol : fragment) {
            if (!cur_node->next.contains(symbol)) {
                cur_node->next[symbol] = new Node;
            }
            cur_node = cur_node->next[symbol];
        }
        cur_node->fragments.emplace_back(p_id, end_pos, fragment.size());
    }

public:
    Trie(const std::vector<Pattern>& patterns) {
        pattern_fragments.resize(patterns.size());
        root = new Node;
        for (size_t p_id = 1; p_id <= patterns.size(); ++p_id) {
            pattern_lengths.push_back(patterns[p_id - 1].symbols.size());
            size_t end_position = 0;
            std::vector<unsigned int> fragment;

            for (const auto& symbol : patterns[p_id - 1].symbols) {
                if (symbol.isJoker) {
                    if (!fragment.empty()) {
                        addFragment(fragment, p_id, end_position);
                        fragment.clear();
                    }
                }
                else {
                    fragment.push_back(symbol.value);
                }
                ++end_position;
            }
            if (!fragment.empty()) {
                addFragment(fragment, p_id, end_position);
            }
        }
        buildLinks();
    }

    ~Trie() {
        delete root;
    }

    Node* getRoot() const {
        return root;
    }

    size_t getPatternFragments(size_t p_id) const {
        return pattern_fragments[p_id - 1];
    }

    size_t getPatternLength(size_t p_id) const {
        return pattern_lengths[p_id - 1];;
    }

    size_t getPatternsNumber() const {
        return pattern_lengths.size();
    }
};

struct Match {
    size_t abs_start;
    size_t votes = 1;

    Match(size_t abs_start) : abs_start(abs_start) {}
};

class Scanner {
    const Trie& trie;
    const Node* cur;

    std::unordered_map<size_t, std::vector<Match>> matches;
    size_t absolute_pos;
    std::vector<size_t> line_starts;

    void saveMatch(size_t absolute_end, const Node* node) {
        for (const auto& f : node->fragments) {
            if (absolute_end <= f.end_position - 1) {
                continue;
            }

            long long abs_start = absolute_end - f.end_position + 1;
            if (!matches.contains(f.pattern_id)) {
                matches[f.pattern_id].emplace_back(abs_start);
            }
            else {
                for (auto it = matches[f.pattern_id].end(); it != matches[f.pattern_id].begin();) {
                    --it;
                    int dist = abs_start - it->abs_start;
                    if (dist == 0) {
                        ++it->votes;
                        break;
                    }
                    // var 1:
                    if (dist > 0) {
                        matches[f.pattern_id].emplace(it + 1, abs_start);
                        break;
                    }
                }
                // var 2:
                // matches[f.pattern_id].emplace_back(abs_start);
            }
        }
    }

public:
    Scanner(const Trie& trie) : trie(trie) {
        cur = trie.getRoot();
        absolute_pos = 0;
        line_starts.push_back(1);
    }

    void feedSymbol(const unsigned int symbol) {
        ++absolute_pos;

        while (!cur->next.contains(symbol) && cur != trie.getRoot()) {
            cur = cur->fail;
        }
        if (cur->next.contains(symbol)) {
            cur = cur->next.find(symbol)->second;
        }
        else {
            cur = trie.getRoot();
        }

        if (cur->isEnd()) {
            saveMatch(absolute_pos, cur);
        }

        Node* out = cur->out;
        while (out) {
            saveMatch(absolute_pos, out);
            out = out->out;
        }
    }

    void feedNewline() {
        line_starts.push_back(absolute_pos + 1);
        return;
    }

    std::pair<size_t, size_t> getLineAndColumn(size_t start_pos) {
        auto it = std::upper_bound(line_starts.begin(), line_starts.end(), start_pos);
        size_t line = std::distance(line_starts.begin(), it);
        size_t line_start = *(--it);
        return {line, start_pos - line_start + 1};
    }

    std::vector<std::vector<std::pair<size_t, size_t>>> getResults(void) {
        std::vector<std::vector<std::pair<size_t, size_t>>> result;
        result.resize(trie.getPatternsNumber());
        for (auto& [pattern_id, v_m] : matches) {
            for (size_t i = 0; i < v_m.size(); ++i) {
                if (v_m[i].votes != trie.getPatternFragments(pattern_id)) {
                    std::swap(v_m[i], v_m.back());
                    v_m.pop_back();
                    --i;
                }
                else if (v_m[i].abs_start + trie.getPatternLength(pattern_id) - 1 <= absolute_pos) {
                    result[pattern_id - 1].push_back(std::move(getLineAndColumn(v_m[i].abs_start)));
                }
            }
        }
        return result;
    }

    size_t getAbsEndPos() const {
        return absolute_pos;
    }
};

class PatternMatcher {
    Trie trie;
    Scanner scanner;
    std::vector<size_t> jokers_id;

    void separatePatterns(const std::vector<Pattern>& patterns) {
        for (int i = 0; i < patterns.size(); ++i) {
            bool all_jokers = true;
            for (const auto& s : patterns[i].symbols) {
                if (!s.isJoker) {
                    all_jokers = false;
                    break;
                }
            }
            if (all_jokers) {
                jokers_id.push_back(i + 1);
            }
        }
    }

public:
    PatternMatcher(const std::vector<Pattern>& patterns) : trie(patterns), scanner(trie) {
        separatePatterns(patterns);
    }

    void feedLine(const std::string& line) {
        std::istringstream iss(std::move(line));
        unsigned num;
        while (iss >> num) {
            scanner.feedSymbol(num);
        }
        scanner.feedNewline();
    }

    void feedSymbol(unsigned int sym) {
        scanner.feedSymbol(sym);
    }

    std::vector<std::vector<std::pair<size_t, size_t>>> getResults() {
        std::vector<std::vector<std::pair<size_t, size_t>>> result(std::move(scanner.getResults()));
        for (const auto& j_id : jokers_id) {
            size_t len = trie.getPatternLength(j_id);
            size_t max_pos = scanner.getAbsEndPos() - len + 1;
            for (size_t start_pos = 1; start_pos <= max_pos; ++start_pos) {
                result[j_id - 1].push_back(std::move(scanner.getLineAndColumn(start_pos)));
            }
        }
        return result;
    }
};

// ========== Генератор тестовых данных ==========
std::pair<std::vector<unsigned int>, std::vector<Symbol>> generate_test(
    size_t text_length,
    size_t pattern_length,
    double joker_prob,
    unsigned int max_value = std::numeric_limits<unsigned int>::max())
{
    std::mt19937 rng(42); // фиксированный seed для воспроизводимости
    std::uniform_int_distribution<unsigned int> val_dist(0, max_value);
    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);

    std::vector<unsigned int> text;
    text.reserve(text_length);
    for (size_t i = 0; i < text_length; ++i)
        text.push_back(val_dist(rng));

    std::vector<Symbol> pattern;
    pattern.reserve(pattern_length);
    for (size_t i = 0; i < pattern_length; ++i) {
        if (prob_dist(rng) < joker_prob)
            pattern.emplace_back(0, true);
        else
            pattern.emplace_back(val_dist(rng), false);
    }
    return {std::move(text), std::move(pattern)};
}

// ========== Наивный поиск образца с джокерами ==========
std::vector<size_t> naive_search(const std::vector<unsigned int>& text,
                                 const std::vector<Symbol>& pattern)
{
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
        if (match)
            occ.push_back(i);
    }
    return occ;
}

// ========== Вспомогательная функция: образец в строку (для конструктора Pattern) ==========
std::string pattern_to_string(const std::vector<Symbol>& pattern) {
    std::ostringstream oss;
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (i != 0) oss << ' ';
        if (pattern[i].isJoker) oss << '?';
        else oss << pattern[i].value;
    }
    return oss.str();
}

// ========== Главная функция бенчмарка ==========
int main() {
    // Параметры тестов
    const std::vector<size_t> text_lengths = {10'000'000, 15'000'000, 20'000'000};
    const size_t pattern_length = 20;
    const double joker_prob = 0.5;
    const unsigned int max_value = 1'000'000; // ограниченный алфавит для более плотных переходов

    std::cout << std::setw(12) << "Text length"
              << std::setw(12) << "Joker %"
              << std::setw(18) << "Build time (s)"
              << std::setw(18) << "AK search (s)"
              << std::setw(18) << "Naive search (s)"
              << std::setw(10) << "Matches"
              << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (auto text_len : text_lengths) {
        // Генерация данных
        auto [text, pattern_syms] = generate_test(text_len, pattern_length, joker_prob, max_value);

        // Строим образец в виде строки (для вашего класса Pattern)
        std::string pat_str = pattern_to_string(pattern_syms);
        std::istringstream pat_stream(pat_str);
        Pattern pattern(pat_stream);
        std::vector<Pattern> patterns = { pattern };

        // Замер времени построения автомата
        auto build_start = std::chrono::high_resolution_clock::now();
        PatternMatcher matcher(patterns);
        auto build_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> build_time = build_end - build_start;

        // Поиск вашим алгоритмом (передача чисел напрямую, если метод feedSymbol доступен)
        // Если добавили метод feedSymbol в PatternMatcher, используйте его.
        // Здесь предполагается, что такой метод есть.
        auto ak_start = std::chrono::high_resolution_clock::now();
        for (auto num : text)
            matcher.feedSymbol(num);
        auto ak_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> ak_search_time = ak_end - ak_start;

        // Получение результатов (1-based позиции) и перевод в 0-based
        auto ak_results = matcher.getResults(); // для первого образца
        std::vector<size_t> ak_occ; // 0-based
        if (!ak_results.empty()) {
            for (auto& match : ak_results[0]) {
                // match = pair<строка, столбец>; столбец = abs_start (1-based)
                ak_occ.push_back(match.second - 1);
            }
            std::sort(ak_occ.begin(), ak_occ.end());
        }

        // Наивный поиск
        auto naive_start = std::chrono::high_resolution_clock::now();
        auto naive_occ = naive_search(text, pattern_syms);
        auto naive_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> naive_time = naive_end - naive_start;

        // Проверка совпадения
        if (ak_occ != naive_occ) {
            std::cerr << "Ошибка: результаты не совпадают для text_len=" << text_len << std::endl;
            return 1;
        }

        // Вывод строки таблицы
        std::cout << std::setw(12) << text_len
                  << std::setw(11) << std::fixed << std::setprecision(0) << (joker_prob * 100) << "%"
                  << std::setw(18) << std::fixed << std::setprecision(6) << build_time.count()
                  << std::setw(18) << ak_search_time.count()
                  << std::setw(18) << naive_time.count()
                  << std::setw(10) << ak_occ.size()
                  << std::endl;
    }

    return 0;
}