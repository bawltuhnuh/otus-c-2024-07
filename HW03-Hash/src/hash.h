#ifndef HASH_H
#define HASH_H

#include <stdlib.h>

struct Element {
    int value;
    char key[100];
};

struct HashTable {
    struct Element** elements;
    long long capacity;
    long long size;
    double load;
};

long long hash_string(const char* str);

struct HashTable* init_table();

void add_element(struct HashTable* table, const char* key, int value);

void remove_element(struct HashTable* table, const char* key);

struct Element* get_element(struct HashTable* table, const char* key);

_Bool is_rehash_required(struct HashTable* table);

void rehash(struct HashTable* table);

#endif // HASH_H
