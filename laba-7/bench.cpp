#include <algorithm>
#include <chrono>
#include <cstdio>
#include <vector>

enum operaion { minus_1, div_2, div_3 };

struct elem {
    int price = 0;
    operaion op = minus_1;
    elem() {}
    elem(int p, operaion o) : price(p), op(o) {}
};

long long solve_dp(int n, std::vector<operaion>* ops = nullptr) {
    std::vector<elem> dp{elem{}, elem{}};
    dp.reserve(n + 1);
    for (int i = 2; i <= n; ++i) {
        dp.emplace_back(dp[i - 1].price, minus_1);
        if (i % 2 == 0 && dp[i / 2].price < dp[i].price)
            dp[i] = {dp[i / 2].price, div_2};
        if (i % 3 == 0 && dp[i / 3].price < dp[i].price)
            dp[i] = {dp[i / 3].price, div_3};
        dp[i].price += i;
    }
    if (ops) {
        ops->clear();
        for (int i = n; i > 1;) {
            ops->push_back(dp[i].op);
            if (dp[i].op == minus_1) --i;
            else if (dp[i].op == div_2) i /= 2;
            else i /= 3;
        }
    }
    return dp[n].price;
}

long long naive(int n) {
    if (n == 1) return 0;
    long long best = naive(n - 1);
    if (n % 2 == 0) best = std::min(best, naive(n / 2));
    if (n % 3 == 0) best = std::min(best, naive(n / 3));
    return best + n;
}

template <class F>
double measure(F f, int repeats) {
    double best = 1e18, sum = 0;
    for (int r = 0; r < repeats; ++r) {
        auto t0 = std::chrono::steady_clock::now();
        volatile long long sink = f();
        (void)sink;
        auto t1 = std::chrono::steady_clock::now();
        double t = std::chrono::duration<double>(t1 - t0).count();
        best = std::min(best, t);
        sum += t;
    }
    return sum / repeats;
}

int main() {
    const int REPEATS = 5;

    std::printf("%-6s %-15s %-12s %-10s\n", "n", "ДП, с", "наивная, с", "во сколько");
    for (int n : {50, 75, 100, 500, 750}) {
        double t_dp = measure([&] { return solve_dp(n); }, REPEATS);
        double t_nv = measure([&] { return naive(n); }, REPEATS);
        std::printf("%-6d %-15.9f %-12.9f %-10.1f\n", n, t_dp, t_nv, t_nv / t_dp);
    }

    std::printf("\n%-6s %-15s \n", "n", "ДП, с");
    for (int n : {1000, 10000, 100000, 1000000, 10000000}) {
        double t_dp = measure([&] { return solve_dp(n); }, REPEATS);
        std::printf("%-6d %-15.9f\n", n, t_dp);
    }

    return 0;
}