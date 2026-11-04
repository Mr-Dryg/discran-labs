#include <iostream>
#include <vector>
#include <array>

int main() {
    int n, m;
    std::cin >> n >> m;
    std::vector<std::array<int, 3>> s(n);
    for (auto& a : s) {
        std::cin >> a[0] >> a[1] >> a[2];
    }
    while (m--) {
        int x, y;
        std::cin >> x >> y;
        int c = 0;
        for (auto& a : s)
            if (a[0] <= x && x <= a[1] && a[2] > y) c++;
        std::cout << c << "\n";
    }
}
