#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

struct Node {
    int left = -1;
    int right = -1;
    int count = 0;
};

std::vector<Node> pool;

int createNode(int l, int r) {
    int id = pool.size();
    pool.push_back(Node());
    if (l == r) {
        return id;
    }
    int mid = (l + r) / 2;
    int left = createNode(l, mid);
    int right = createNode(mid + 1, r);
    pool[id].left = left;
    pool[id].right = right;
    return id;
}

int add(int old_node_id, int l, int r, int pos, int delta) {
    int new_node_id = pool.size();
    pool.push_back(Node());
    pool[new_node_id].count = pool[old_node_id].count + delta;
    if (l == r) {
        return new_node_id;
    }
    int mid = (l + r) / 2;
    int left, right;
    if (pos <= mid) {
        left = add(pool[old_node_id].left, l, mid, pos, delta);
        right = pool[old_node_id].right;
    } else {
        left = pool[old_node_id].left;
        right = add(pool[old_node_id].right, mid + 1, r, pos, delta);
    }
    pool[new_node_id].left = left;
    pool[new_node_id].right = right;
    return new_node_id;
}

int count_height(int node_id, int l, int r, int pos) {
    int c = 0;
    int mid = (l + r) / 2;
    while (l < pos) {
        if (pos <= mid) {
            r = mid;
            c += pool[pool[node_id].right].count;
            node_id = pool[node_id].left;
        } else {
            l = mid + 1;
            node_id = pool[node_id].right;
        }
        mid = (l + r) / 2;
    }
    return c + pool[node_id].count;
}

struct Event {
    int x, h, delta;
};

int main(void) {
    int n, m;

    std::cin >> n >> m;

    std::vector<Event> events;
    std::vector<int> heights;
    events.reserve(2 * n);
    for (int i = 0; i < n; ++i) {
        int l, r, h;
        std::cin >> l >> r >> h;
        events.push_back({l, h, 1});
        events.push_back({r+1, h, -1});
        heights.push_back(h);
    }

    std::sort(
        events.begin(),
        events.end(),
        [](const auto& a, const auto& b) {return a.x < b.x;}
    );

    std::sort(heights.begin(), heights.end());
    heights.erase(
        std::unique(heights.begin(), heights.end()),
        heights.end()
    );

    std::vector<int> roots;
    roots.reserve(2 * n + 1);
    roots.push_back(createNode(0, n-1));
    for (auto& event : events) {
        auto it = std::lower_bound(heights.begin(), heights.end(), event.h);
        roots.push_back(add(
            roots.back(),
            0,
            n-1,
            std::distance(heights.begin(), it),
            event.delta
        ));
    }

    for (int i = 0; i < m; ++i) {
        int x, y;
        std::cin >> x >> y;

        auto it_event = std::upper_bound(
            events.begin(),
            events.end(),
            x,
            [](const auto& val, const auto& b){return val < b.x;}
        );
        auto v = std::distance(events.begin(), it_event);

        auto it_height = std::upper_bound(heights.begin(), heights.end(), y);
        if (it_height == heights.end()) {
            std::cout << 0 << '\n';
            continue;
        }
        auto pos = std::distance(heights.begin(), it_height);

        std::cout << count_height(roots[v], 0, n-1, pos) << '\n';
    }
}
