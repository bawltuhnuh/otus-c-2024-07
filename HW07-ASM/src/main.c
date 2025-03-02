#include <stdio.h>
#include <stdlib.h>

// TODO: переписать m и f.
// оптимизация m и f в asm
// исправление утечки памяти в asm

char* int_format = "%ld ";

struct Node {
    long long int value;
    struct Node* prev;
};

void print_int(long long int value) {
    printf(int_format, value);
    fflush(NULL);
}


struct Node* add_element(long long int value, struct Node* prev) {
    struct Node* node = (struct Node*) malloc(sizeof(struct Node));
    node->value = value;
    node->prev = prev;
    return node;
}

// aka m - в asm реализована как функция высшего порядка типа map
void print_list(struct Node* node) {
    if (node == NULL) {
        puts("");
        return;
    }
    print_int(node->value);
    print_list(node->prev);
}

// aka f - в asm реализована как функция высшего порядка типа filter
struct Node* filter_by_last_digit(struct Node* node) {
    struct Node* cur_node = node;
    struct Node* prev = NULL;
    while (cur_node != NULL) {
        if (cur_node->value & 1) {
            prev = add_element(cur_node->value, prev);
        }
        cur_node = cur_node->prev;
    }
    return prev;
}

int main(int argc, char** argv) {
    long long int data[] = {4, 8, 15, 16, 23, 42};
    size_t data_length = sizeof(data)/sizeof(long long int);
    struct Node* prev = NULL;
    for (size_t i = data_length; i > 0; --i) {
        prev = add_element(data[i - 1], prev);
    }
    print_list(prev);
    struct Node* filtered = filter_by_last_digit(prev);
    print_list(filtered);
}
