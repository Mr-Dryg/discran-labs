#include <iostream>
#include <vector>

struct item {
    int weight;
    int price;

    item(int w, int p) : weight(w), price(p) {}
};

struct bag {
    int sum_price;
    std::vector<int> items{};

    bag() : sum_price(-1), items() {}
};

int main(void) {
    int n, capacity;
    std::cin >> n >> capacity;

    std::vector<item> items;
    items.reserve(n);

    int w, p;
    for (int i = 0; i < n; ++i) {
        std::cin >> w >> p;
        items.emplace_back(w, p);
    }

    // dp[k][w] = bag
    // k - число выбранных предметов
    // w - суммарный вес выбранных предметов
    // bag хранит макисмальную общую стоимость и добавленные предметы
    std::vector<std::vector<bag>> dp(n + 1);
    for (int k = 0; k <= n; ++k) {
        dp[k].resize(capacity + 1);
    }
    dp[0][0].sum_price = 0;

    for (int i = 0; i < n; ++i) {
        for (int k = i + 1; k > 0; --k) {
            for (int w = capacity; w >= items[i].weight; --w) {
                if (dp[k - 1][w - items[i].weight].sum_price < 0) {
                    continue;
                }
                if (dp[k][w].sum_price < dp[k - 1][w - items[i].weight].sum_price + items[i].price) {
                    dp[k][w].sum_price = dp[k - 1][w - items[i].weight].sum_price + items[i].price;
                    dp[k][w].items = dp[k - 1][w - items[i].weight].items;
                    dp[k][w].items.push_back(i);
                }
            }
        }
    }

    int max_res = 0;
    std::vector<int>* res_items = &dp[0][0].items;
    for (int k = 0; k <= n; ++k) {
        for (int w = 0; w <= capacity; ++w) {
            if (max_res < dp[k][w].sum_price * k) {
                max_res = dp[k][w].sum_price * k;
                res_items = &dp[k][w].items;
            }
        }
    }

    std::cout << max_res << '\n';

    for (auto& i: *res_items) {
        std::cout << i + 1 << ' ';
    }

    return 0;
}