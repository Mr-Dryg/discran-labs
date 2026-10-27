#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

#define MAX_PRICE 50
#define EPS 1e-10

std::vector<int> counting_sort_input(std::vector<std::vector<double>>& v, std::vector<std::vector<double>>& res) {
    std::vector<int> count(MAX_PRICE + 1);
    for (int i = 0; i < v.size(); ++i) {
        ++count[v[i].back()];
    }
    for (int i = 1; i < count.size(); ++i) {
        count[i] += count[i - 1];
    }
    std::vector<int> permutations(v.size());
    res.resize(v.size());
    for (int i = v.size() - 1; i >= 0; --i) {
        int j = --count[v[i].back()];
        res[j] = std::move(v[i]);
        permutations[j] = i;
    }
    return permutations;
}

std::vector<int> counting_sort_ans(std::vector<int>& v) {
    std::vector<int> count(1 + *std::max_element(v.begin(), v.end()));
    for (int i = 0; i < v.size(); ++i) {
        ++count[v[i]];
    }
    for (int i = 1; i < count.size(); ++i) {
        count[i] += count[i - 1];
    }
    std::vector<int> res(v.size());
    for (int i = v.size() - 1; i >= 0; --i) {
        int j = --count[v[i]];
        res[j] = v[i];
    }
    return res;
}

int main(void) {
    int m, n;
    std::cin >> m >> n;

    std::vector<std::vector<double>> input;
    input.resize(m);
    for (int i = 0; i < m; ++i) {
        input[i].resize(n + 1);
        for (int j = 0; j <= n; ++j) {
            std::cin >> input[i][j];
        }
    }

    std::vector<std::vector<double>> bads;
    auto permutations = counting_sort_input(input, bads);
    input.clear();

    std::vector<int> basis; // [индекс базисного элемента] = индекс строки
    basis.resize(n, -1);

    for (int i = 0; i < m; ++i) {
        bool full_basis = true;
        // вычитаем из новой строки все что есть в базисе
        for (int k = 0; k < n; ++k) {
            full_basis &= (basis[k] != -1);

            if (std::abs(bads[i][k]) < EPS) {
                continue;
            }

            if (basis[k] == -1) {
                basis[k] = i;
                for (int j = n-1; j >= k; --j) {
                    bads[i][j] /= bads[i][k];
                }
                break;
            }

            for (int j = n-1; j >= k; --j) {
                bads[i][j] -= bads[i][k] * bads[basis[k]][j];
            }
        }

        if (full_basis) {
            break;
        }
    }

    std::vector<int> ans;
    for (int i = 0; i < n; ++i) {
        if (basis[i] == -1) {
            break;
        }
        ans.push_back(permutations[basis[i]] + 1);
    }

    if (ans.size() < n) {
        std::cout << -1;
    } else {
        ans = counting_sort_ans(ans);
        for (auto& i : ans) {
            std::cout << i << " ";
        }
    }
}