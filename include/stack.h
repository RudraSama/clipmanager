#ifndef STACK_H_
#define STACK_H_

#include <stdint.h>

struct Node {
	char *data;
	uint64_t offset;
	uint32_t hash;
	unsigned long data_size;
	struct Node *next;
};

typedef struct Node Node;

typedef struct {
	Node *bottom;
	Node *top;
} Stack;

Stack *stack_init();

void stack_deinit(Stack **stack);

void push(Stack **stack, const char *data, uint64_t offset, uint32_t hash,
          unsigned long data_size);

void pop(Stack **stack);

void traverse(Stack **stack);

#endif
