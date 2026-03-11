#include <iostream>
#include <random>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <n>\n";
        return 1;
    }

    int n = std::atoi(argv[1]);
    if (n <= 0) {
        std::cerr << "n должно быть положительным числом\n";
        return 1;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> day_dist(1, 31);
    std::uniform_int_distribution<> month_dist(1, 12);
    std::uniform_int_distribution<> year_dist(0, 9999);
    std::uniform_int_distribution<> len_dist(1, 64);
    std::uniform_int_distribution<> char_size_dist(0, 1);
    std::uniform_int_distribution<> char_dist(0, 25);

    for (int i = 0; i < n; ++i) {
        int day = day_dist(gen);
        int month = month_dist(gen);
        int year = year_dist(gen);
        int str_len = len_dist(gen);

        std::string str;
        str.reserve(str_len);

        for (int j = 0; j < str_len; ++j) {
            int r = char_dist(gen);
            char start;
            if (char_size_dist(gen)) {
                start = 'A';
            } else {
                start = 'a';
            }
            str += start + r;
        }

        std::cout << day << '.' << month << '.' << year << '\t' << str << '\n';
    }

    return 0;
}