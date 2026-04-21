#ifndef SKIP_LIST_HPP
#define SKIP_LIST_HPP

#include <vector>
#include <cstdlib>
#include <ctime>

// Generic skip list that only requires operator< on T.
// Equality is defined as !(a<b) && !(b<a)

template<typename T>
class SkipList {
private:
    struct Node {
        T* value;                 // nullptr for head; otherwise new T(v)
        std::vector<Node*> next;  // forward pointers per level
        explicit Node(int lvl) : value(nullptr), next(lvl, nullptr) {}
        Node(const T& v, int lvl) : value(new T(v)), next(lvl, nullptr) {}
        ~Node() { delete value; }
    };

    static constexpr int MAX_LEVEL = 32;
    Node* head;   // dummy head node
    int level;    // current max level in use [1..MAX_LEVEL]

    static bool lt(const T& a, const T& b) { return a < b; }
    static bool eqv(const T& a, const T& b) { return !lt(a,b) && !lt(b,a); }

    int randomLevel() {
        int lvl = 1;
        while (lvl < MAX_LEVEL && (std::rand() & 1)) ++lvl; // ~p=0.5
        return lvl;
    }

public:
    SkipList() : head(new Node(MAX_LEVEL)), level(1) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }

    ~SkipList() {
        Node* cur = head->next[0];
        while (cur) {
            Node* nxt = cur->next[0];
            delete cur;
            cur = nxt;
        }
        delete head;
    }

    // Insert a value into the skip list. If the value already exists, do nothing.
    void insert(const T & item) {
        std::vector<Node*> update(MAX_LEVEL, nullptr);
        Node* cur = head;
        for (int i = level - 1; i >= 0; --i) {
            while (cur->next[i] && lt(*cur->next[i]->value, item)) cur = cur->next[i];
            update[i] = cur;
        }
        Node* candidate = cur->next[0];
        if (candidate && eqv(*candidate->value, item)) return; // exists

        int newLevel = randomLevel();
        if (newLevel > level) {
            for (int i = level; i < newLevel; ++i) update[i] = head;
            level = newLevel;
        }
        Node* node = new Node(item, newLevel);
        for (int i = 0; i < newLevel; ++i) {
            node->next[i] = update[i]->next[i];
            update[i]->next[i] = node;
        }
    }

    // Search for a value in the skip list
    bool search(const T & item) {
        Node* cur = head;
        for (int i = level - 1; i >= 0; --i) {
            while (cur->next[i] && lt(*cur->next[i]->value, item)) cur = cur->next[i];
        }
        cur = cur->next[0];
        return cur && eqv(*cur->value, item);
    }

    // Delete a value from the skip list. If the value does not exist in the skip list, do nothing.
    void deleteItem(const T & item) {
        std::vector<Node*> update(MAX_LEVEL, nullptr);
        Node* cur = head;
        for (int i = level - 1; i >= 0; --i) {
            while (cur->next[i] && lt(*cur->next[i]->value, item)) cur = cur->next[i];
            update[i] = cur;
        }
        cur = cur->next[0];
        if (!cur || !eqv(*cur->value, item)) return; // not found

        for (int i = 0; i < level; ++i) {
            if (update[i]->next[i] != cur) break;
            update[i]->next[i] = cur->next[i];
        }
        delete cur;

        while (level > 1 && head->next[level - 1] == nullptr) --level;
    }
};

#endif
