#ifndef HASHTABLE_H_INCLUDED
#define HASHTABLE_H_INCLUDED

/* in order to get strdup(), this needs to be defined */
#define _BSD_SOURCE

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct list_node {
    const char *k;
    void *v;
    struct list_node *next;
} list_node_t;

typedef struct hash_table {
    list_node_t **t;
    size_t sz;
} hash_table_t;

uint32_t hash_str(const char *s);
list_node_t * list_add(list_node_t *head, const char *k, void *v);
list_node_t * list_remove(list_node_t *head, const char *k);
list_node_t * list_get(list_node_t *head, const char *k);
size_t list_length(list_node_t *head);

hash_table_t * hashtable_new(size_t size);
void hash_table_free(hash_table_t *ht);

static inline void hashtable_add(hash_table_t *ht, const char *name, void *c) {
    uint32_t h = hash_str(name) % ht->sz;
    ht->t[h] = list_add(ht->t[h], name, (void *)c);
} 

static inline void hashtable_remove(hash_table_t *ht, const char *name) {
    uint32_t h = hash_str(name) % ht->sz;
    ht->t[h] = list_remove(ht->t[h], name);
}

static inline void * hashtable_get(hash_table_t *ht, const char *name) {
    uint32_t h = hash_str(name) % ht->sz;
    list_node_t *n = list_get(ht->t[h], name);
    return n ? n->v : NULL;
}

#endif // HASHTABLE_H_INCLUDED

