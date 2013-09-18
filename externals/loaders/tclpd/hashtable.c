#include "hashtable.h"

uint32_t hash_str(const char *s) {
    const unsigned char *p = (const unsigned char *)s;
    uint32_t h = 5381;

    while (*p) {
        h *= 33;
        h ^= *p++;
    }

    return h ^ (h >> 16);
}

list_node_t * list_add(list_node_t *head, const char *k, void *v) {
    list_node_t *n = (list_node_t *)malloc(sizeof(list_node_t));
    n->next = head;
#ifdef HASHTABLE_COPY_KEYS
    n->k = strdup(k);
#else
    n->k = k;
#endif
    n->v = v;
    return n;
}

list_node_t * list_remove(list_node_t *head, const char *k) {
    if(!head) return NULL;

    list_node_t *tmp;

    // head remove
    while(head && strcmp(head->k, k) == 0) {
        tmp = head;
        head = head->next;
#ifdef HASHTABLE_COPY_KEYS
        free(tmp->k);
#endif
        free(tmp);
    }

    list_node_t *p = head;

    // normal (non-head) remove
    while(p->next) {
        if(strcmp(p->next->k, k) == 0)
        {
            tmp = p->next;
            p->next = p->next->next;
#ifdef HASHTABLE_COPY_KEYS
            free(tmp->k);
#endif
            free(tmp);
            continue;
        }
        p = p->next;
    }

    return head;
}

list_node_t * list_get(list_node_t *head, const char *k) {
    while(head) {
        if(strcmp(head->k, k) == 0) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

size_t list_length(list_node_t *head) {
    size_t length = 0;
    while(head) {
        length++;
        head = head->next;
    }
    return length;
}

hash_table_t * hashtable_new(size_t size) {
    hash_table_t *ht = NULL;
    if(size > 0) {
        ht = (hash_table_t *)malloc(sizeof(hash_table_t));
        ht->sz = size;
        ht->t = (list_node_t **)malloc(sizeof(list_node_t *) * size);
	for(int i = 0; i < size; i++) ht->t[i] = NULL;
    }
    return ht;
}

void hashtable_free(hash_table_t *ht) {
    if(ht) {
        free(ht->t);
        free(ht);
    }
}

