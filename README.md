skiplist.h
==========

Single-header-file, public domain, type-generic C89 skip list implementation.
-----------------------------------------------------------------------------

1. Download [skiplist.h](https://raw.githubusercontent.com/alpha123/skiplist.h/master/skiplist.h).
   That's the only file you need.
2. Create a guarded header that defines SKIPLIST_KEY and SKIPLIST_VALUE
   and includes skiplist.h. Optionally define SKIPLIST_NAMESPACE.
   Define SKIPLIST_IMPLEMENTATION somewhere in your program above
   your custom header include.
3. Repeat for any different key/value pair types you need. Be sure to
   define different SKIPLIST_NAMESPACE values and define SKIPLIST_IMPLEMENTATION
   once for each key/value type pair.
 
Other options:

 - SKIPLIST_MAX_LEVELS - 33 by default.
 - SKIPLIST_MALLOC & SKIPLIST_FREE - wrappers for stdlib malloc/free by default.
   Both are passed a void \* data pointer (for memory pool, gc context, etc).
 - SKIPLIST_RAND & SKIPLIST_SRAND - wrappers around stdlib rand/srand.
   Both are passed a void \* pointer for a random context.
 - SKIPLIST_STATIC - if defined, declare all public functions static
   (make skiplist local to the file it's included from).
 - SKIPLIST_EXTERN - 'extern' by default; define to change calling convention
   or linkage etc.

Example Usage
-------------

```c
/* skiplist_str_int.h */
/* This header should be guarded, as below */
#ifndef SKIPLIST_STR_INT_H
#define SKIPLIST_STR_INT_H

#define SKIPLIST_KEY const char *
#define SKIPLIST_VALUE int
#define SKIPLIST_NAMESPACE sl_strint_
#include "skiplist.h"

#endif

/* program.c */
#include <stdio.h>
#include <string.h>
/* You should only define this once. If it helps,
   you can simply make a skiplist_str_int.c file
   with the following 2 lines and link to it. */
#define SKIPLIST_IMPLEMENTATION
#include "skiplist_str_int.h"

int cmp(const char *a, const char *b, void *userdata) {
    return strcmp(a, b);
}

int iter(const char *key, int val, void *userdata) {
    printf("%s = %d\n", key, val);
    return 0;
}

int main(int argc, const char **argv) {
    sl_strint_skiplist list;
    int err = sl_strint_init(&list, cmp, NULL);
    // Not real error handling
    if (err) {
        puts("Uh oh");
        exit(err);
    }
    
    sl_strint_insert(&list, "a", 1, NULL);
    sl_strint_insert(&list, "c", 3, NULL);
    sl_strint_insert(&list, "b", 2, NULL);
    
    short has_b = sl_strint_find(&list, "b", NULL),  // => 1
          has_d = sl_strint_find(&list, "d", NULL);  // => 0
    
    int a_val;
    short exists = sl_strint_find(&list, "a", &a_val);
    if (exists)
        printf("a = %d\n", a_val);
    else
        puts("a does not exist");

    int default_val = 10;
    int d_val = sl_strint_get(&list, "d", default_val);  // => 10
    
    sl_strint_iter(&list, iter, NULL);
    // Prints a = 1, b = 2, c = 3

    int b_val;
    short existed = sl_strint_remove(&list, "b", &b_val);
    if (existed)
        print("b used to be %d, but now it is no more\n", b_val);
    else
        puts("b was only an illusion, a fleeting glimpse of a dream");
    
    sl_strint_free(&list);
    return 0;
}
```

License
-------
       
Where possible, this software has been disclaimed from any copyright
and is placed in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy
and modify this file in any way you see fit.
