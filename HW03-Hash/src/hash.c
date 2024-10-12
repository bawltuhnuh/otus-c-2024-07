#include "hash.h"

#include <string.h>
#include <math.h>

const int p = 53;

long long hash_string(const char* str) {
    size_t str_len = strlen(str);
    long long hash = 0, p_pow = 1;
    for (size_t i = 0; i < str_len; ++i) {
        hash += abs(str[i] - 'a' + 1) * p_pow;
        p_pow *= p;
    }
    return hash;
}

struct HashTable* init_table()
{
    struct HashTable* table = (struct HashTable*)malloc(sizeof(struct HashTable));
    table->capacity = 10;
    table->size = 0;
    table->load = 0.7;
    table->elements = (struct Element**)malloc(sizeof(struct Element*)*table->capacity);
    for (long long i = 0; i < table->capacity; ++i) {
        table->elements[i] = NULL;
    }
    return table;
}

_Bool is_rehash_required(struct HashTable *table)
{
    double current_load = table->size * 1.0 / table->capacity;
    return current_load > table->load;
}

struct Element *get_element(struct HashTable* table, const char* key)
{
    long long hash = hash_string(key) % table->capacity;
    struct Element* current_element = table->elements[hash];
    while (current_element != NULL && strcmp(current_element->key, key) != 0) {
        ++hash;
        current_element = table->elements[hash];
    }
    return current_element;
}

void remove_element(struct HashTable *table, const char *key)
{
    long long hash = hash_string(key) % table->capacity;
    struct Element* current_element = table->elements[hash];
    while (current_element != NULL && current_element->key != key) {
        ++hash;
        current_element = table->elements[hash];
    }
    if (current_element != NULL) {
        free(current_element);
        table->elements[hash] = NULL;
        table->size -= 1;
    }
}

void add_element(struct HashTable *table, const char* key, int value)
{
    long long hash = hash_string(key) % table->capacity;
    struct Element* current_element = table->elements[hash];
    while (current_element != NULL) {
        hash = (hash + 1) % table->capacity;
        current_element = table->elements[hash];
    }
    table->elements[hash] = (struct Element*)malloc(sizeof(struct Element));
    strcpy(table->elements[hash]->key, key);
    table->elements[hash]->value = value;
    ++table->size;
    if (is_rehash_required(table)) {
        rehash(table);
    }
}

void rehash(struct HashTable *table)
{
    if (table->capacity == 0) {
        return;
    }
    long long new_capacity = table->capacity * 2;
    struct Element** new_elements = (struct Element**) malloc(sizeof(struct Element*) * new_capacity);
    for (long long i = 0; i < new_capacity; ++i) {
        new_elements[i] = NULL;
    }
    for (long long i = 0; i < table->capacity; ++i) {
        struct Element* element = table->elements[i];
        if (element != NULL) {
            long long hash = hash_string(element->key) % new_capacity;
            new_elements[hash] = element;
        }
    }
    free(table->elements);
    table->capacity = new_capacity;
    table->elements = new_elements;
}
