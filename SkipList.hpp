#ifndef SKIP_LIST_HPP
#define SKIP_LIST_HPP

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <climits>

// A generic skip list implementation that requires only operator< on T
// Equality is determined via !(a < b) && !(b < a)

template<typename T>
class SkipList {
private:
    struct Node {
        T value;
        std::vector<Node*> forward;
        explicit Node(const T& v, int level) : value(v), forward(level, nullptr) {}
    };

    // Configuration parameters
    static constexpr int MAX_LEVEL = 32; // enough for up to ~2^32 elements theoretically
    static constexpr double P = 0.5;     // promotion probability

    Node* head;   // head with MAX_LEVEL forwards
    int level;    // current highest level (1..MAX_LEVEL)

    static bool lessThan(const T& a, const T& b) {
        return a < b;
    }
    static bool equalTo(const T& a, const T& b) {
        return !lessThan(a, b) && !lessThan(b, a);
    }

    int randomLevel() {
        int lvl = 1;
        while (lvl < MAX_LEVEL && (std::rand() & 1)) { // fast P ~ 0.5
            ++lvl;
        }
        return lvl;
    }

public:
    SkipList() : head(new Node(T{}, MAX_LEVEL)), level(1) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }

    ~SkipList() {
        // delete all nodes level-0 chain
        Node* cur = head->forward[0];
        while (cur) {
            Node* nxt = cur->forward[0];
            delete cur;
            cur = nxt;
        }
        delete head;
    }

    // Insert a value into the skip list. If the value already exists, do nothing.
    void insert(const T & item) {
        std::vector<Node*> update(MAX_LEVEL, nullptr);
        Node* cur = head;

        // find positions to update
        for (int i = level - 1; i >= 0; --i) {
            while (cur->forward[i] && lessThan(cur->forward[i]->value, item)) {
                cur = cur->forward[i];
            }
            update[i] = cur;
        }

        Node* next = cur->forward[0];
        if (next && equalTo(next->value, item)) {
            // already exists, do nothing
            return;
        }

        int newLevel = randomLevel();
        if (newLevel > level) {
            for (int i = level; i < newLevel; ++i) {
                update[i] = head;
            }
            level = newLevel;
        }

        Node* node = new Node(item, newLevel);
        for (int i = 0; i < newLevel; ++i) {
            node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = node;
        }
    }

    // Search for a value in the skip list
    bool search(const T & item) {
        Node* cur = head;
        for (int i = level - 1; i >= 0; --i) {
            while (cur->forward[i] && lessThan(cur->forward[i]->value, item)) {
                cur = cur->forward[i];
            }
        }
        cur = cur->forward[0];
        return cur && equalTo(cur->value, item);
    }

    // Delete a value from the skip list. If the value does not exist in the skip list, do nothing.
    void deleteItem(const T & item) {
        std::vector<Node*> update(MAX_LEVEL, nullptr);
        Node* cur = head;

        for (int i = level - 1; i >= 0; --i) {
            while (cur->forward[i] && lessThan(cur->forward[i]->value, item)) {
                cur = cur->forward[i];
            }
            update[i] = cur;
        }

        cur = cur->forward[0];
        if (!cur || !equalTo(cur->value, item)) {
            return; // not found
        }

        for (int i = 0; i < level; ++i) {
            if (update[i]->forward[i] != cur) break;
            update[i]->forward[i] = cur->forward[i];
        }
        delete cur;

        while (level > 1 && head->forward[level - 1] == nullptr) {
            --level;
        }
    }
};

#endif
