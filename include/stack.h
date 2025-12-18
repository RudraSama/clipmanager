#ifndef STACK_H_
#define STACK_H_

#include <stdint.h>

struct Node {
	char *data;
	uint32_t hash;
	unsigned long data_size;
	struct Node *next;
};

typedef struct Node Node;

typedef struct {
	Node *bottom;
	Node *top;
} Stack;

void push(Stack **stack, const char *data, unsigned long data_size);

void pop(Stack **stack);

void traverse(Stack **stack);

#endif
