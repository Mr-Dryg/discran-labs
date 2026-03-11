#include <cstdio>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <array>
#include <functional>

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

void counting_sort(std::vector<std::pair<Date, std::string>>& unsorted_data, int radix);

void radix_sort(std::vector<std::pair<Date, std::string>>& data);

void read_sort_write(std::function<void(std::vector<std::pair<Date, std::string>>&)> sort);
