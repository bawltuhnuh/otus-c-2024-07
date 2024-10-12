#include <stdio.h>
#include "hash.h"

void print_usage() {
    printf("Usage: hash <path>");
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        print_usage();
        return 1;
    }

    FILE* f = fopen(argv[1], "rb");
    if (f == NULL) {
        printf("Open error");
        return 1;
    }

    char word[100];

    struct HashTable* table = init_table();

    while (fscanf(f, "%99s", word) == 1) {
        struct Element* element = get_element(table, word);
        if (element == NULL) {
            add_element(table, word, 1);
        } else {
            element->value += 1;
        }
    }

    fclose(f);
    for (long long i = 0; i < table->capacity; ++i) {
        struct Element* element = table->elements[i];
        if (element != NULL) {
            printf("%s %d\n", element->key, element->value);
        }
    }

    return 0;
}
