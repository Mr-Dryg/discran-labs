#include <cstdio>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <array>

#define CHAR_FOR_DAYS 2
#define CHAR_FOR_MONTH 2
#define CHAR_FOR_YEARS 4
#define DIGITS_NUM 10

struct Date {
    std::string row_date;
    std::string sort_key;
    
    friend std::istream& operator>>(std::istream& is, Date& date);
    friend std::ostream& operator<<(std::ostream& os, const Date& date);
};

std::istream& operator>>(std::istream& is, Date& date) {
    is >> date.row_date;

    char day[CHAR_FOR_DAYS + 1];
    char month[CHAR_FOR_MONTH + 1];
    char year[CHAR_FOR_YEARS + 1];
    sscanf(date.row_date.c_str(), "%[^.].%[^.].%[^ \t]", day, month, year);

    std::string day_str {day};
    std::string month_str {month};
    std::string year_str {year};

    for (int i = day_str.length(); i < CHAR_FOR_DAYS; ++i) {
        day_str = '0' + day_str;
    }

    for (int i = month_str.length(); i < CHAR_FOR_MONTH; ++i) {
        month_str = '0' + month_str;
    }

    for (int i = year_str.length(); i < CHAR_FOR_YEARS; ++i) {
        year_str = '0' + year_str;
    }

    date.sort_key = year_str + month_str + day_str;

    return is;
}

std::ostream& operator<<(std::ostream& os, const Date& date) {
    os << date.row_date;
    return os;
}

void counting_sort(std::vector<std::pair<Date, std::string>>& unsorted_data, int radix) {
    std::array<int, DIGITS_NUM> pref_sums {0};

    for (const auto& obj : unsorted_data) {
        int i = obj.first.sort_key[radix] - '0';
        ++pref_sums[i];
    }

    for (int i = 1; i < DIGITS_NUM; ++i) {
        pref_sums[i] += pref_sums[i - 1];
    }

    std::vector<std::pair<Date, std::string>> sorted_data {unsorted_data.size()};

    for (int j = unsorted_data.size() - 1; j >= 0; --j) {
        int i = unsorted_data[j].first.sort_key[radix] - '0';
        sorted_data[--pref_sums[i]] = std::move(unsorted_data[j]);
    }

    unsorted_data = std::move(sorted_data);
}

void radix_sort(std::vector<std::pair<Date, std::string>>* data) {
    for (int radix = CHAR_FOR_DAYS + CHAR_FOR_MONTH + CHAR_FOR_YEARS - 1; radix >= 0; --radix) {
        counting_sort(*data, radix);
    }
}

int main(void) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);


    std::vector<std::pair<Date, std::string>> data;
    Date date;
    std::string value;

    while (std::cin >> date >> value) {
        data.emplace_back(date, value);
    }

    radix_sort(&data);

    for (const auto& obj : data) {
        std::cout << obj.first << '\t' << obj.second << '\n';
    }

    return 0;
}