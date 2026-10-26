#include <iostream>
#include <string>

int main(void) {
    int n, m;
    std::cin >> n >> m;
    std::string s = std::to_string(n);

    int res = 0;
    int l = 1;
    int r = s[0] - '0';
    for (int k = 1; k <= s.length(); ++k) {
        res += r / m - (l - 1) / m;
        l *= 10;
        r = r * 10 + s[k] - '0';
    }

    if (n % m == 0) {
        --res;
    }

    std::cout << res;
}