#ifndef STACK_H_
#define STACK_H_

#include <clip_metadata.h>
#include <stdint.h>

struct Node {
	ClipMetadata_t clip;
	struct Node *next;
};

typedef struct Node Node;

typedef struct {
	Node *bottom;
	Node *top;
} Stack;

Stack *stack_init();

void stack_deinit(Stack **stack);

void push(Stack **stack, const ClipMetadata_t *clip);

void pop(Stack **stack);

void traverse(Stack **stack);

#endif
