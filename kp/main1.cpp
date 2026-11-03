#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

struct Node {
    Node* left = nullptr;
    Node* right = nullptr;
    int count = 0;
};

Node* createNode(int l, int r) {
    Node* root = new Node();
    if (l == r) {
        return root;
    }
    int mid = (l + r) / 2;
    root->left = createNode(l, mid);
    root->right = createNode(mid + 1, r);
    return root;
}

Node* add(Node* old_node, int l, int r, int pos, int delta) {
    Node* new_node = new Node();
    new_node->count = old_node->count + delta;
    if (l == r) {
        return new_node;
    }
    int mid = (l + r) / 2;
    if (pos <= mid) {
        new_node->left = add(old_node->left, l, mid, pos, delta);
        new_node->right = old_node->right;
    } else {
        new_node->left = old_node->left;
        new_node->right = add(old_node->right, mid + 1, r, pos, delta);
    }
    return new_node;
}

int count_height(Node* node, int l, int r, int pos) {
    int c = 0;
    int mid = (l + r) / 2;
    while (l < pos) {
        if (pos <= mid) {
            r = mid;
            c += node->right->count;
            node = node->left;
        } else {
            l = mid + 1;
            node = node->right;
        }
        mid = (l + r) / 2;
    }
    return c + node->count;
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

    std::vector<Node*> roots;
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
