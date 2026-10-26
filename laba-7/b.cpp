#include <iostream>
#include <vector>

int main(void) {
    int n, m;

    std::cin >> n >> m;

    std::vector<std::vector<int>> input;
    input.resize(n);
    for (int i = 0; i < n; ++i) {
        input[i].resize(m);
        for (int j = 0; j < m; ++j) {
            char num;
            std::cin >> num;
            input[i][j] = num - '0';
        }
    }

    // dp[i][j] = s
    // в i строке в j столбце идет s нулей (считаем сверху вниз)

    std::vector<std::vector<int>> dp;
    dp.resize(n+1);

    dp[0].resize(m);

    for (int i = 1; i <= n; ++i) {
        dp[i].resize(m);
        for (int j = 0; j < m; ++j) {
            if (input[i-1][j] == 0) {
                dp[i][j] += dp[i-1][j] + 1;
            }
        }
    }

    int res_max = 0;

    for (int i = n; i > 0; --i) {
        std::vector<int> heights;
        std::vector<int> left_border;
        left_border.resize(m);

        int k;
        for (int j = 0; j < m; ++j) {
            while (heights.size() && dp[i][j] < dp[i][k = heights.back()]) {
                res_max = std::max(res_max, dp[i][k] * (j - left_border[k]));
                heights.pop_back();
            }

            if (heights.size() == 0) {
                left_border[j] = 0;
                heights.push_back(j);
            } else if (dp[i][j] > dp[i][k = heights.back()]) {
                left_border[j] = k + 1;
                heights.push_back(j);
            } else if (dp[i][j] == dp[i][k]) {
                left_border[j] = left_border[k];
                heights.push_back(j);
            }
        }        

        for (auto& j : heights) {
            res_max = std::max(res_max, dp[i][j] * (m - left_border[j]));
        }
    }

    std::cout << res_max;

    return 0;
}