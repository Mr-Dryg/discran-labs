#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <utility>
#include <vector>

#define MAX_PRICE 50
#define EPS 1e-10

using Matrix = std::vector<std::vector<double>>;

std::vector<int> counting_sort_input(Matrix& v, Matrix& res) {
    std::vector<int> count(MAX_PRICE + 1);
    for (int i = 0; i < (int)v.size(); ++i) {
        ++count[(int)v[i].back()];
    }
    for (int i = 1; i < (int)count.size(); ++i) {
        count[i] += count[i - 1];
    }
    std::vector<int> permutations(v.size());
    res.resize(v.size());
    for (int i = (int)v.size() - 1; i >= 0; --i) {
        int j = --count[(int)v[i].back()];
        res[j] = std::move(v[i]);
        permutations[j] = i;
    }
    return permutations;
}

std::vector<int> counting_sort_ans(std::vector<int>& v) {
    std::vector<int> count(1 + *std::max_element(v.begin(), v.end()));
    for (int i = 0; i < (int)v.size(); ++i) {
        ++count[v[i]];
    }
    for (int i = 1; i < (int)count.size(); ++i) {
        count[i] += count[i - 1];
    }
    std::vector<int> res(v.size());
    for (int i = (int)v.size() - 1; i >= 0; --i) {
        int j = --count[v[i]];
        res[j] = v[i];
    }
    return res;
}

std::vector<int> solve_greedy(Matrix input, int n) {
    int m = (int)input.size();
    Matrix bads;
    auto permutations = counting_sort_input(input, bads);

    std::vector<int> basis(n, -1);
    for (int i = 0; i < m; ++i) {
        bool full_basis = true;
        for (int k = 0; k < n; ++k) {
            full_basis &= (basis[k] != -1);
            if (std::abs(bads[i][k]) < EPS) continue;
            if (basis[k] == -1) {
                basis[k] = i;
                for (int j = n - 1; j >= k; --j) bads[i][j] /= bads[i][k];
                break;
            }
            for (int j = n - 1; j >= k; --j) bads[i][j] -= bads[i][k] * bads[basis[k]][j];
        }
        if (full_basis) break;
    }

    std::vector<int> ans;
    for (int i = 0; i < n; ++i) {
        if (basis[i] == -1) break;
        ans.push_back(permutations[basis[i]] + 1);
    }
    if ((int)ans.size() < n) return {};
    return counting_sort_ans(ans);
}

int rank_of(std::vector<std::vector<double>> a, int n) {
    int rows = (int)a.size(), r = 0;
    for (int col = 0; col < n && r < rows; ++col) {
        int p = r;
        for (int i = r + 1; i < rows; ++i)
            if (std::abs(a[i][col]) > std::abs(a[p][col])) p = i;
        if (std::abs(a[p][col]) < EPS) continue;
        std::swap(a[p], a[r]);
        for (int i = r + 1; i < rows; ++i) {
            double f = a[i][col] / a[r][col];
            for (int j = col; j < n; ++j) a[i][j] -= f * a[r][j];
        }
        ++r;
    }
    return r;
}

// наивный перебор всех подмножеств добавок
long long solve_brute(const Matrix& in, int n) {
    int m = (int)in.size();
    long long best = -1;
    for (unsigned mask = 1; mask < (1u << m); ++mask) {
        if (std::popcount(mask) < n) continue;
        long long cost = 0;
        for (int i = 0; i < m; ++i)
            if (mask >> i & 1) cost += (long long)in[i].back();
        if (best != -1 && cost >= best) continue;
        std::vector<std::vector<double>> rows;
        for (int i = 0; i < m; ++i)
            if (mask >> i & 1) rows.push_back(in[i]);
        if (rank_of(rows, n) == n) best = cost;
    }
    return best;
}

// генерация данных
Matrix gen(std::mt19937& rng, int m, int n, int max_coef, int max_price, bool deficient) {
    std::uniform_int_distribution<int> coef(0, max_coef), price(0, max_price);
    Matrix a(m, std::vector<double>(n + 1));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) a[i][j] = coef(rng);
        if (deficient) a[i][n - 1] = 0; // последний столбец нулевой: ранг < n
        a[i][n] = price(rng);
    }
    return a;
}

long long total_cost(const Matrix& a, const std::vector<int>& ans) {
    long long s = 0;
    for (int idx : ans) s += (long long)a[idx - 1].back();
    return s;
}

template <class F>
double measure(F f, int repeats) {
    double sum = 0;
    for (int r = 0; r < repeats; ++r) {
        auto t0 = std::chrono::steady_clock::now();
        volatile long long sink = f();
        (void)sink;
        auto t1 = std::chrono::steady_clock::now();
        sum += std::chrono::duration<double>(t1 - t0).count();
    }
    return sum / repeats;
}

int main() {
    std::mt19937 rng(12345);

    // 1. корректность
    const int TESTS = 3000;
    int bad = 0, impossible = 0;
    for (int t = 0; t < TESTS; ++t) {
        int n = 1 + (int)(rng() % 5);
        int m = n + (int)(rng() % 6);
        Matrix a = gen(rng, m, n, 2, 10, false);
        auto ans = solve_greedy(a, n);
        long long ref = solve_brute(a, n);
        bool ok;
        if (ans.empty()) {
            ok = (ref == -1);
            ++impossible;
        } else {
            std::vector<std::vector<double>> rows;
            for (int idx : ans) rows.push_back(a[idx - 1]);
            ok = (ref == total_cost(a, ans)) && (rank_of(rows, n) == n) && ((int)ans.size() == n);
        }
        if (!ok) {
            ++bad;
            std::printf("MISMATCH test %d (m=%d, n=%d)\n", t, m, n);
        }
    }
    std::printf("Correctness vs brute force: %s (%d random tests, %d of them with answer -1)\n\n",
                bad == 0 ? "OK" : "FAILED", TESTS, impossible);

    // 2. жадный алгоритм против перебора подмножеств (N = 5, M растёт).
    std::printf("%-4s %-4s %-16s %-16s %-12s\n", "M", "N", "greedy, s", "brute force, s", "ratio");
    for (int m : {8, 12, 16, 20}) {
        int n = 5;
        Matrix a = gen(rng, m, n, 50, 50, false);
        double tg = measure([&] { auto r = solve_greedy(a, n); return (long long)r.size(); }, 3);
        double tb = measure([&] { return solve_brute(a, n); }, 3);
        std::printf("%-4d %-4d %-16.9f %-16.9f %-12.1f\n", m, n, tg, tb, tb / tg);
    }

    // 3. масштабирование жадного алгоритма по N (M = 2N).
    //    full rank  : ранг полный, остановка, как только набран базис;
    //    rank N-1   : последний столбец нулевой, обрабатываются все M строк.
    std::printf("\n%-6s %-6s %-16s %-16s %-10s %-14s\n",
                "N", "M", "full rank, s", "rank N-1, s", "growth", "ns/(M*N^2)");
    double prev = 0;
    for (int n : {50, 100, 200, 400, 800}) {
        int m = 2 * n;
        Matrix full = gen(rng, m, n, 50, 50, false);
        Matrix defi = gen(rng, m, n, 50, 50, true);
        double t1 = measure([&] { return (long long)solve_greedy(full, n).size(); }, 3);
        double t2 = measure([&] { return (long long)solve_greedy(defi, n).size(); }, 3);
        double unit = t2 / ((double)m * n * n) * 1e9;
        if (prev > 0)
            std::printf("%-6d %-6d %-16.6f %-16.6f x%-9.2f %-14.3f\n", n, m, t1, t2, t2 / prev, unit);
        else
            std::printf("%-6d %-6d %-16.6f %-16.6f %-10s %-14.3f\n", n, m, t1, t2, "-", unit);
        prev = t2;
    }
    return 0;
}