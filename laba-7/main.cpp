#include <iostream>
#include <vector>

enum operaion {
    minus_1,
    div_2,
    div_3
};

struct elem {
    int price = 0;
    operaion op;

    elem() {}
    elem(int p, operaion o) : price(p), op(o) {}
};

int main(void) {
    int n;
    std::cin >> n;

    std::vector<elem> dp {elem{}, elem{}};
    dp.reserve(n+1);

    for (int i = 2; i <= n; ++i) {
        dp.emplace_back(dp[i-1].price, minus_1);
        if (i % 2 == 0 && dp[i / 2].price < dp[i].price) {
            dp[i] = {dp[i / 2].price, div_2};
        }
        if (i % 3 == 0 && dp[i / 3].price < dp[i].price) {
            dp[i] = {dp[i / 3].price, div_3};
        }
        dp[i].price += i;
    }

    std::cout << dp[n].price << '\n';

    for (int i = n; i > 1;) {
        if (dp[i].op == minus_1) {
            std::cout << "-1 ";
            --i;
        } else if (dp[i].op == div_2) {
            std::cout << "/2 ";
            i /= 2;
        } else if (dp[i].op == div_3) {
            std::cout << "/3 ";
            i /= 3;
        }
    }
}