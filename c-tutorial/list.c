#include "list.h"

void list_init(list_t *l, int (*compare)(const void *key, const void *with), void (*datum_delete)(void *datum)) {
    l->head = NULL;
    l->tail = NULL;
    l->length = 0;
    l->compare = compare;
    l->datum_delete = datum_delete;
}

void list_visit_items(list_t *l, void (*visitor)(void *v)) {
    list_item_t *current = l->head;
    while (current) {
        visitor(current->datum);
        current = current->next;
    }
}

void list_insert_tail(list_t *l, void *v) {
    list_item_t *new_item = malloc(sizeof(list_item_t));
    if (!new_item) {
        perror("malloc for list item");
        exit(EXIT_FAILURE);
    }

    new_item->datum = v;
    new_item->next = NULL;

    if (!l->head) {
        l->head = new_item;
        l->tail = new_item;
    } else {
        l->tail->next = new_item;
        l->tail = new_item;
    }
}

void list_remove_head(list_t *l) {
    if (!l->head) return; // List is empty

    list_item_t *temp = l->head;
    l->head = l->head->next;
    free(temp);

    if (!l->head)
        l->tail = NULL;
}

// Visitor function to print each line
void print_line(void *datum) {
    char *line = (char *)datum;
    printf("%s\n", line);
}

//check if the list is empty
bool is_list_empty(list_t *l) {
    return l->head == NULL;
}