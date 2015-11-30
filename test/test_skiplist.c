#include "ptest.h"

#define SKIPLIST_KEY int
#define SKIPLIST_VALUE int
#define SKIPLIST_IMPLEMENTATION
#include "../skiplist.h"

int int_cmp(int a, int b, void *_udata) {
    return a - b;
}

#define SETUP sl_skiplist sl; sl_init(&sl, int_cmp, NULL, NULL, NULL);
#define TEARDOWN sl_free(&sl);

#define TEST(name) void test_ ## name(void) { SETUP
#define END(name) TEARDOWN }

TEST(insert)
    int old;
    PT_ASSERT(sl_insert(&sl, 3, 3, NULL) == 0);
    PT_ASSERT(sl_insert(&sl, 1, 3, NULL) == 0);
    PT_ASSERT(sl_insert(&sl, 3, 7, &old) == 1);
    PT_ASSERT(old == 3);
    PT_ASSERT(sl_insert(&sl, 5, 5, &old) == 0);
    PT_ASSERT(sl_insert(&sl, 3, 1, &old) == 1);
    PT_ASSERT(old == 7);
END(insert)

TEST(find)
    int val;
    PT_ASSERT(sl_find(&sl, 10, NULL) == 0);
    sl_insert(&sl, 10, 4, NULL);
    sl_insert(&sl, 4, 10, NULL);
    sl_insert(&sl, 8, 8, NULL);
    PT_ASSERT(sl_find(&sl, 10, &val) == 1);
    PT_ASSERT(val == 4);
    sl_insert(&sl, 8, 2, NULL);
    PT_ASSERT(sl_find(&sl, 8, &val) == 1);
    PT_ASSERT(val == 2);
END(find)

TEST(get)
    PT_ASSERT(sl_get(&sl, 2, 10) == 10);
    sl_insert(&sl, 4, 4, NULL);
    PT_ASSERT(sl_get(&sl, 4, 10) == 4);
END(get)

TEST(size)
    PT_ASSERT(sl_size(&sl) == 0);
    sl_insert(&sl, 1, 2, NULL);
    PT_ASSERT(sl_size(&sl) == 1);
    sl_insert(&sl, 2, 2, NULL);
    sl_insert(&sl, 1, 4, NULL);
    PT_ASSERT(sl_size(&sl) == 2);
END(size)

struct iter_data {
    int cnt;
    int keys[5];
    int vals[5];
};

int int_iter(int k, int v, void *data) {
    struct iter_data *id = (struct iter_data *)data;
    id->keys[id->cnt] = k;
    id->vals[id->cnt++] = v;
    return 0;
}

TEST(iter)
    struct iter_data id;
    id.cnt = 0;
    PT_ASSERT(sl_iter(&sl, int_iter, &id) == 0);
    PT_ASSERT(id.cnt == 0);

    sl_insert(&sl, 2, 10, NULL);
    sl_insert(&sl, 3, 3, NULL);
    sl_insert(&sl, 1, 5, NULL);
    sl_insert(&sl, 2, 4, NULL);
    sl_insert(&sl, 5, 1, NULL);
    sl_insert(&sl, 4, 2, NULL);

    PT_ASSERT(sl_iter(&sl, int_iter, &id) == 0);
    PT_ASSERT(id.cnt == 5);
    for (int i = 0; i < 5; ++i) {
        PT_ASSERT(id.keys[i] == i+1);
        PT_ASSERT(id.vals[i] == 5-i);
    }
END(iter)

void suite_skiplist(void) {
    pt_add_test(test_insert, "Should insert key/value pairs", "");
    pt_add_test(test_find, "Should find values that exist", "");
    pt_add_test(test_get, "Should be able to return a default value for keys that don't exist", "");
    pt_add_test(test_size, "Should keep track of its size", "");
    pt_add_test(test_iter, "Should iterate over keys in order", "");
}

int main(int argc, const char **argv) {
    pt_add_suite(suite_skiplist);
    return pt_run();
}
