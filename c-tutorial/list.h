#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct list_item {
    struct list_item *prev, *next;
    void *datum;
} list_item_t;

typedef struct list {
    list_item_t *head, *tail;
    unsigned length;
    int (*compare)(const void *key, const void *with);
    void (*datum_delete)(void *);
} list_t;

// Function prototypes
void list_init(list_t *l, int (*compare)(const void *key, const void *with), void (*datum_delete)(void *datum));
void list_visit_items(list_t *l, void (*visitor)(void *v));
void list_insert_tail(list_t *l, void *v);
void list_remove_head(list_t *l);
void print_line(void *datum);
bool is_list_empty(list_t *l);

#endif
