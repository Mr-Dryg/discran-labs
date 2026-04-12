#include "rbtree.h"
#include <chrono>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <iomanip>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<double, std::milli>;

std::string random_string(size_t length, std::mt19937& gen) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);
    std::string str(length, 0);
    for (size_t i = 0; i < length; ++i) {
        str[i] = charset[dist(gen)];
    }
    return str;
}

void test_my_rbt(size_t num_operations) {
    RBTree tree;
    std::mt19937 gen(42);
    std::vector<std::string> keys;

    keys.reserve(num_operations);
    for (size_t i = 0; i < num_operations; ++i) {
        keys.push_back(random_string(10, gen));
    }

    std::cout << "\nRBTree:" << std::endl;

    TimePoint start = Clock::now();
    for (size_t i = 0; i < num_operations; ++i) {
        tree.insert(keys[i], i);
    }
    Duration insert_time = Clock::now() - start;
    std::cout << "  Insert: " << std::fixed << std::setprecision(3) 
              << insert_time.count() << " ms" << std::endl;

    start = Clock::now();
    for (size_t i = 0; i < num_operations; ++i) {
        tree.find(keys[i]);
    }
    Duration find_time = Clock::now() - start;
    std::cout << "  Find:   " << std::fixed << std::setprecision(3) 
              << find_time.count() << " ms" << std::endl;

    start = Clock::now();
    for (size_t i = 0; i < num_operations; ++i) {
        tree.remove(keys[i]);
    }
    Duration remove_time = Clock::now() - start;
    std::cout << "  Remove: " << std::fixed << std::setprecision(3) 
              << remove_time.count() << " ms" << std::endl;
}

void test_std_map(size_t num_operations) {
    std::map<std::string, unsigned long long> map;
    std::mt19937 gen(42);
    std::vector<std::string> keys;

    keys.reserve(num_operations);
    for (size_t i = 0; i < num_operations; ++i) {
        keys.push_back(random_string(10, gen));
    }

    std::cout << "\nstd::map:" << std::endl;

    TimePoint start = Clock::now();
    for (size_t i = 0; i < num_operations; ++i) {
        map[keys[i]] = i;
    }
    Duration insert_time = Clock::now() - start;
    std::cout << "  Insert: " << std::fixed << std::setprecision(3) 
              << insert_time.count() << " ms" << std::endl;

    start = Clock::now();
    for (size_t i = 0; i < num_operations; ++i) {
        map.find(keys[i]);
    }
    Duration find_time = Clock::now() - start;
    std::cout << "  Find:   " << std::fixed << std::setprecision(3) 
              << find_time.count() << " ms" << std::endl;

    start = Clock::now();
    for (size_t i = 0; i < num_operations; ++i) {
        map.erase(keys[i]);
    }
    Duration remove_time = Clock::now() - start;
    std::cout << "  Remove: " << std::fixed << std::setprecision(3) 
              << remove_time.count() << " ms" << std::endl;
}

void run_comparison(size_t num_operations) {
    std::cout << "\n=== Comparison for " << num_operations << " operations ===" << std::endl;
    test_my_rbt(num_operations);
    test_std_map(num_operations);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number_of_operations>" << std::endl;
        return 1;
    }

    try {
        size_t num_ops = std::stoull(argv[1]);
        run_comparison(num_ops);
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid number - " << e.what() << std::endl;
        return 1;
    }

    return 0;
}