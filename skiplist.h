/* Where possible, this software has been disclaimed from any copyright
   and is placed in the public domain. Where that dedication is not
   recognized, you are granted a perpetual, irrevocable license to copy
   and modify this file in any way you see fit. */

/**
 * 1. Create a guarded header that defines SKIPLIST_KEY and SKIPLIST_VALUE
 *    and includes skiplist.h. Optionally define SKIPLIST_NAMESPACE.
 * 2. Define SKIPLIST_IMPLEMENTATION somewhere in your program above
 *    your custom header include.
 * 3. Repeat for any different key/value pair types you need. Be sure to
 *    define different SKIPLIST_NAMESPACE values and define SKIPLIST_IMPLEMENTATION
 *    once for each key/value type pair.
 * 4. Other options:
 *      - SKIPLIST_MAX_LEVELS - 33 by default
 *      - SKIPLIST_MALLOC & SKIPLIST_FREE - wrappers for stdlib malloc/free by default
 *        Both are passed a void * data pointer (for memory pool, gc context, etc).
 *      - SKIPLIST_RAND & SKIPLIST_SRAND - wrappers around stdlib rand/srand.
 *        Both are passed a void * pointer for a random context.
 *      - SKIPLIST_STATIC - if defined, declare all public functions static
 *        (make skiplist local to the file it's included from).
 *      - SKIPLIST_EXTERN - 'extern' by default; define to change calling convention
 *        or linkage etc.
 *
 * Example:
 *
 *     // skiplist_str_int.h
 *     // Include this file wherever you need a string -> int skiplist
 *     #ifndef SKIPLIST_STR_INT_H
 *     #define SKIPLIST_STR_INT_H
 *     
 *     #define SKIPLIST_KEY const char *
 *     #define SKIPLIST_VALUE int
 *     #define SKIPLIST_NAMESPACE sl_strint_
 *     #include "skiplist.h"
 *
 *     #endif
 *
 *     // program.c
 *     // short test drive program
 *     #include <stdio.h>
 *     #include <string.h>
 *     #define SKIPLIST_IMPLEMENTATION
 *     #include "skiplist_str_int.h"
 *
 *     int cmp(const char *a, const char *b, void *userdata) {
 *         return strcmp(a, b);
 *     }
 *
 *     int iter(const char *key, int val, void *userdata) {
 *         printf("%s = %d\n", key, val);
 *         return 0;
 *     }
 *
 *     int main(int argc, const char **argv) {
 *         sl_strint_skiplist list;
 *         int err = sl_strint_init(&list, cmp, NULL);
 *         // Not real error handling
 *         if (err) {
 *             puts("Uh oh");
 *             exit(err);
 *         }
 *         
 *         sl_strint_insert(&list, "a", 1, NULL);
 *         sl_strint_insert(&list, "c", 3, NULL);
 *         sl_strint_insert(&list, "b", 2, NULL);
 *         
 *         short has_b = sl_strint_find(&list, "b", NULL),  // => 1
 *               has_d = sl_strint_find(&list, "d", NULL);  // => 0
 *         
 *         int a_val;
 *         short exists = sl_strint_find(&list, "a", &a_val);
 *         if (exists)
 *             printf("a = %d\n", a_val);
 *         else
 *             puts("a does not exist");
 *
 *         int default_val = 10;
 *         int d_val = sl_strint_get(&list, "d", default_val);  // => 10
 *         
 *         sl_strint_iter(&list, iter, NULL);
 *         // Prints a = 1, b = 2, c = 3
 *
 *         int b_val;
 *         short existed = sl_strint_remove(&list, "b", &b_val);
 *         if (existed)
 *             print("b used to be %d, but now it is no more\n", b_val);
 *         else
 *             puts("b was only an illusion, a fleeting glimpse of a dream");
 *         
 *         sl_strint_free(&list);
 *         return 0;
 *     }
 */

#ifndef SKIPLIST_MALLOC
#ifdef SKIPLIST_IMPLEMENTATION
#include <stdlib.h>
#endif
#define SKIPLIST_MALLOC(udata, sz) malloc((sz))
#define SKIPLIST_FREE(udata, ptr) free((ptr))
#endif

#if !defined(SKIPLIST_KEY) || !defined(SKIPLIST_VALUE)
#error Please define SKIPLIST_KEY and SKIPLIST_VALUE before including \
this file. See the comments at the top for usage instructions.
#endif

#ifndef SKIPLIST_NAMESPACE
#define SKIPLIST_NAMESPACE sl_
#endif

#if !defined(SKIPLIST_EXTERN) && !defined(SKIPLIST_STATIC)
#define SKIPLIST_EXTERN extern
#elif defined(SKIPLIST_STATIC)
#define SKIPLIST_EXTERN static
#endif

#ifndef SKIPLIST_MAX_LEVELS
#define SKIPLIST_MAX_LEVELS 33
#endif

#define SL_PASTE_(x,y) x ## y
#define SL_CAT_(x,y) SL_PASTE_(x,y)
#define SKIPLIST_NAME(name) SL_CAT_(SKIPLIST_NAMESPACE,name)

#define SL_NODE SKIPLIST_NAME(node)
#define SL_LIST SKIPLIST_NAME(skiplist)
#define SL_CMP_FN SKIPLIST_NAME(cmp_fn)
#define SL_ITER_FN SKIPLIST_NAME(iter_fn)
#define SL_KEY SKIPLIST_KEY
#define SL_VAL SKIPLIST_VALUE

#ifndef SKIPLIST_RAND
#include <stdlib.h>
#include <time.h>
void SKIPLIST_NAME(_stdsrand)(void *_userdata);
#ifdef SKIPLIST_IMPLEMENTATION
void SKIPLIST_NAME(_stdsrand)(void *_userdata) {
    time_t t;
    srand((unsigned)time(&t));
}
#endif
#define SKIPLIST_RAND(udata) rand()
#define SKIPLIST_SRAND(udata) SKIPLIST_NAME(_stdsrand)(udata)
#endif

typedef int (* SL_CMP_FN)(SL_KEY, SL_KEY, void *);
typedef int (* SL_ITER_FN)(SL_KEY, SL_VAL, void *);

typedef struct SKIPLIST_NAME(_node) {
    unsigned int height;
    SL_KEY key;
    SL_VAL val;
    struct SKIPLIST_NAME(_node) *prev;
    struct SKIPLIST_NAME(_node) *next[SKIPLIST_MAX_LEVELS];
} SL_NODE;

typedef struct {
    unsigned long size;
    unsigned int highest;
    SL_CMP_FN cmp;
    void *cmp_udata;
    void *mem_udata;
    void *rand_udata;
    SKIPLIST_NAME(node) *head;
} SL_LIST;

/* Must be called prior to using any other functions on a skiplist.
 * @list a pointer to the skiplist to initialize
 * @cmp the comparator function to use to order nodes
 * @cmp_udata Opaque pointer to pass to cmp
 * @mem_udata Opaque pointer to pass to the SKIPLIST_MALLOC and
 *            SKIPLIST_FREE macros. Unused by default, but custom
  *           memory allocators may use it.
 * @rand_udata Opaque pointer to pass to the SKIPLIST_RAND and
 *             SKIPLIST_SRAND macros. Unused by default, but custom
 *             RNGs may use it.
 *
 * @return 0 if successful and nonzero if something failed
 */
SKIPLIST_EXTERN
int SKIPLIST_NAME(init)(SL_LIST *list, SL_CMP_FN cmp, void *cmp_udata, void *mem_udata, void *rand_udata);

/* Free memory used by a skiplist.
 * @list Free this guy from his bondage to memory.
 */
SKIPLIST_EXTERN
void SKIPLIST_NAME(free)(SL_LIST *list);

/* Sets a value in the skiplist.
 * @list An initialized skiplist
 * @key Associate the value with this key
 * @val Value
 * @prior If non-NULL, put the prior value at this location. Not touched if
 *        there was no prior value associated with this key.
 *
 * If a value already exists at that key,
 * overwrite it and stick the prior value in this function's out parameter.
 *
 * @return 0 if no value was at this key, 1 if a value did exist and was
 *          overwritten.
 */
SKIPLIST_EXTERN
short SKIPLIST_NAME(insert)(SL_LIST *list, SL_KEY key, SL_VAL val, SL_VAL *prior);

/* Gets a value associated with a key.
 * @list An initialized skiplist
 * @key Get the value associated with this key
 * @out If a value exists, store it at this location.
 *      If this parameter is NULL, nothing is stored.
 *
 * @return 0 if the key does not exist, 1 if it does
 */
SKIPLIST_EXTERN
short SKIPLIST_NAME(find)(SL_LIST *list, SL_KEY key, SL_VAL *out);

/* Gets a value associated with a key, or a default value.
 * @list An initialized skiplist
 * @key Get the value associated with this key
 * @default_val If the key does not exist in this list,
 *              return this value instead.
 *
 * @return The value associated with the key or default_val if the
 *          key is not set.
 */
SKIPLIST_EXTERN
SL_VAL SKIPLIST_NAME(get)(SL_LIST *list, SL_KEY key, SL_VAL default_val);

/* Removes a key/value pair from this list.
 * @list An initialized skiplist
 * @key Key indicating the key/value pair to remove
 * @out If non-NULL and the key existed, store the old value at this location
 *
 * @return 1 if the key used to be in the list (and was thus removed),
 *          0 if it was never there
 */
SKIPLIST_EXTERN
short SKIPLIST_NAME(remove)(SL_LIST *list, SL_KEY key, SL_VAL *out);

/* Iterates through all key/value pairs in order.
 * @list An initialized skiplist
 * @iter An iterator function to call for each key/value pair
 * @userdata An opaque pointer to pass to `iter`.
 *
 * If `iter` returns a non-zero value, stop the iteration and return
 * that value.
 *
 * @return The first non-zero result of `iter` or 0 if `iter` always
 *         returned 0.
 */
SKIPLIST_EXTERN
int SKIPLIST_NAME(iter)(SL_LIST *list, SL_ITER_FN iter, void *userdata);

/* Does what it says on the tin.
 * @list An initialized skiplist
 *
 * @return The number of key/value pairs in the skiplist
 */
SKIPLIST_EXTERN
unsigned long SKIPLIST_NAME(size)(SL_LIST *list);

/* Returns the minimum key and value in this list.
 * @list An initalized skiplist
 * @key_out Set to the smallest key if non-NULL and the list is not empty
 * @val_out Set to the value associated with the smallest key if non-NULL
 *          and the list is not empty.
 *
 * @return 0 if the list is empty and 1 if it is not
 */
SKIPLIST_EXTERN
short SKIPLIST_NAME(min)(SL_LIST *list, SL_KEY *key_out, SL_VAL *val_out);

/* Returns the maximum key and value in this list.
 * @list An initalized skiplist
 * @key_out Set to the largest key if non-NULL and the list is not empty
 * @val_out Set to the value associated with the largest key if non-NULL
 *          and the list is not empty.
 *
 * @return 0 if the list is empty and 1 if it is not
 */
SKIPLIST_EXTERN
short SKIPLIST_NAME(max)(SL_LIST *list, SL_KEY *key_out, SL_VAL *val_out);

/* Removes and returns the minimum key/value pair from a list.
 * @list An initialized skiplist
 * @key_out Set to the smallest key if non-NULL and the list is not empty
 * @val_out Set to the value associated with the smallest key if non-NULL
 *          and the list is not empty.
 *
 * @return 0 if the list was already empty and 1 if it was not
 */
SKIPLIST_EXTERN
short SKIPLIST_NAME(pop)(SL_LIST *list, SL_KEY *key_out, SL_VAL *val_out);

/* Removes and returns the maximum key/value pair from a list.
 * @list An initialized skiplist
 * @key_out Set to the largest key if non-NULL and the list is not empty
 * @val_out Set to the value associated with the largest key if non-NULL
 *          and the list is not empty.
 *
 * @return 0 if the list was already empty and 1 if it was not
 */
SKIPLIST_EXTERN
short SKIPLIST_NAME(shift)(SL_LIST *list, SL_KEY *key_out, SL_VAL *val_out);

#ifdef SKIPLIST_IMPLEMENTATION

SKIPLIST_EXTERN
int SKIPLIST_NAME(init)(SL_LIST *list, SL_CMP_FN cmp, void *cmp_udata, void *mem_udata, void *rand_udata) {
    list->cmp = cmp;
    list->cmp_udata = cmp_udata;
    list->mem_udata = mem_udata;
    list->rand_udata = rand_udata;
    SKIPLIST_SRAND(rand_udata);
    list->highest = 0;
    list->size = 0;
    list->head = (SL_NODE *)SKIPLIST_MALLOC(mem_udata, sizeof(SL_NODE));
    list->head->height = SKIPLIST_MAX_LEVELS;
    memset(list->head->next, 0, SKIPLIST_MAX_LEVELS * sizeof(SL_NODE *));
    return 0;
}

SKIPLIST_EXTERN
void SKIPLIST_NAME(free)(SL_LIST *list) {
    SL_NODE *n, *next;
    n = list->head;
    while (n) {
        next = n->next[0];
        SKIPLIST_FREE(list->mem_udata, n);
        n = next;
    }
}

SKIPLIST_EXTERN
short SKIPLIST_NAME(insert)(SL_LIST *list, SL_KEY key, SL_VAL val, SL_VAL *prior) {
    SL_NODE *n, *nn, *update[SKIPLIST_MAX_LEVELS];
    int r;
    unsigned int i;
    short replaced;

    n = list->head;
    nn = (SL_NODE *)SKIPLIST_MALLOC(list->mem_udata, sizeof(SL_NODE));
    nn->key = key;
    nn->val = val;
    memset(nn->next, 0, SKIPLIST_MAX_LEVELS * sizeof(SL_NODE *));

    for (r = 0, i = 0; !(r & 1); ++i) {
        if (r == 0)
            r = SKIPLIST_RAND(list->rand_udata);
        r >>= 1;
    }
    nn->height = i;

    i = list->highest;
    while (i --> 0) {
        while (n->next[i] && list->cmp(key, n->next[i]->key, list->cmp_udata) > 0)
            n = n->next[i];
        update[i] = n;
    }

    replaced = n->next[0] != NULL && list->cmp(key, n->next[0]->key, list->cmp_udata) == 0;
    if (replaced) {
        if (prior)
            *prior = n->next[0]->val;
        n->next[0]->val = val;
        SKIPLIST_FREE(list->mem_udata, nn);
    }
    else {
        while (nn->height > list->highest)
            update[list->highest++] = list->head;

        i = nn->height;
        while (i --> 0) {
            nn->next[i] = update[i]->next[i];
            update[i]->next[i] = nn;
        }
    }

    list->size += !replaced;

    return replaced;
}

SKIPLIST_EXTERN
short SKIPLIST_NAME(find)(SL_LIST *list, SL_KEY key, SL_VAL *out) {
    SL_NODE *n;
    int cmp;
    unsigned int i;
    n = list->head;
    i = list->highest;

    while (i --> 0) {
        while (n->next[i]) {
            if ((cmp = list->cmp(key, n->next[i]->key, list->cmp_udata)) == 0)
                goto found;
            else if (cmp < 0)
                break;
            n = n->next[i];
        }
    }
    return 0;

    found:
    if (out)
        *out = n->next[i]->val;
    return 1;
}

SKIPLIST_EXTERN
SL_VAL SKIPLIST_NAME(get)(SL_LIST *list, SL_KEY key, SL_VAL default_val) {
    SL_VAL v;
    if (SKIPLIST_NAME(find)(list, key, &v))
        return v;
    return default_val;
}

SKIPLIST_EXTERN
short SKIPLIST_NAME(remove)(SL_LIST *list, SL_KEY key, SL_VAL *out) {
    SL_NODE *n;
    SL_NODE *update[SKIPLIST_MAX_LEVELS];
    int cmp;
    unsigned int i;
    n = list->head;
    i = list->highest;

    while (i --> 0) {
        while (n->next[i] && (cmp = list->cmp(n->next[i]->key, key, list->cmp_udata)) < 0) {
            n = n->next[i];
        }
        update[i] = n;
    }

    n = n->next[0];
    if (n && (list->cmp(n->key, key, list->cmp_udata) == 0) && out) {
      *out = n->val;
      i = 0;
      while (i < list->highest) {
        if (update[i]->next[i] != n) break;
        update[i]->next[i] = n->next[i];
        i++;
      }
      SKIPLIST_FREE(list->mem_udata, n);
      while (list->highest > 0 && list->head->next[list->highest - 1] == NULL) {
        --list->highest;
      }
      --list->size;
      return 1;
    }
    return 0;
}

SKIPLIST_EXTERN
int SKIPLIST_NAME(iter)(SL_LIST *list, SL_ITER_FN iter, void *userdata) {
    SL_NODE *n;
    int stop;
    n = list->head;
    while (n->next[0]) {
        if ((stop = iter(n->next[0]->key, n->next[0]->val, userdata)))
            return stop;
        n = n->next[0];
    }
    return 0;
}

SKIPLIST_EXTERN
unsigned long SKIPLIST_NAME(size)(SL_LIST *list) {
    return list->size;
}

SKIPLIST_EXTERN
short SKIPLIST_NAME(min)(SL_LIST *list, SL_KEY *key_out, SL_VAL *val_out) {
    if (list->size == 0)
        return 0;
    if (key_out)
        *key_out = list->head->next[0]->key;
    if (val_out)
        *val_out = list->head->next[0]->val;
    return 1;
}

SKIPLIST_EXTERN
short SKIPLIST_NAME(max)(SL_LIST *list, SL_KEY *key_out, SL_VAL *val_out) {
    int i;
    SL_NODE *n;
    if (list->size == 0)
        return 0;
    /* TODO store the biggest */
    n = list->head;
    for (i = 0; i < list->size; ++i)
        n = n->next[0];
    if (key_out)
        *key_out = n->key;
    if (val_out)
        *val_out = n->val;
    return 1;
}

SKIPLIST_EXTERN
short SKIPLIST_NAME(pop)(SL_LIST *list, SL_KEY *key_out, SL_VAL *val_out) {
    int i;
    SL_NODE *first;

    if (list->size == 0)
        return 0;

    first = list->head->next[0];
    i = first->height + 1;
    while (i --> 0) {
        if (list->head->next[i])
            list->head->next[i] = first->next[i];
        else
            --list->highest;
    }

    if (key_out)
        *key_out = first->key;
    if (val_out)
        *val_out = first->val;
    SKIPLIST_FREE(list->mem_udata, first);
    --list->size;
    return 1;
}

SKIPLIST_EXTERN
short SKIPLIST_NAME(shift)(SL_LIST *list, SL_KEY *key_out, SL_VAL *val_out) {
    int i;
    SL_NODE *penultimate, *last;
    if (list->size == 0)
        return 0;
    penultimate = list->head;
    for (i = 0; i < list->size - 1; ++i)
        penultimate = penultimate->next[0];
    last = penultimate->next[0];
    if (key_out)
        *key_out = last->key;
    if (val_out)
        *val_out = last->val;
    i = penultimate->height;
    while (i --> 0)
        penultimate->next[i] = NULL;
    SKIPLIST_FREE(list->mem_udata, last);
    --list->size;
    return 1;
}

#endif

#undef SL_PASTE_
#undef SL_CAT_

#undef SL_NODE
#undef SL_LIST
#undef SL_CMP_FN
#undef SL_ITER_FN
#undef SL_KEY
#undef SL_VAL
