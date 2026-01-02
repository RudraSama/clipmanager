#include <stack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Stack *stack_init() {
	Stack *stack = malloc(sizeof(Stack));
	if (stack == NULL) return NULL;

	stack->top = NULL;
	stack->bottom = NULL;
	return stack;
}

void stack_deinit(Stack **stack) {
	Node *curr = (*stack)->bottom;
	while (curr != NULL) {
		Node *temp = curr->next;
		free(curr->data);
		free(curr);
		curr = temp;
	}
	free(*stack);
}

void push(Stack **stack, const char *data, uint64_t offset, uint32_t hash,
          unsigned long data_size) {
	if (*stack == NULL) return;

	Node *new_node = malloc(sizeof(Node));
	if (!new_node) return;
	new_node->next = NULL;
	new_node->hash = hash;
	new_node->offset = offset;
	new_node->data_size = data_size;
	new_node->data = strdup(data);

	if (new_node->data == NULL) {
		free(new_node);
		return;
	}

	if ((*stack)->bottom == NULL || (*stack)->top == NULL) {
		(*stack)->top = new_node;
		(*stack)->bottom = new_node;
		return;
	}

	(*stack)->top->next = new_node;
	(*stack)->top = new_node;
}

void pop(Stack **stack) {
	if (*stack == NULL || (*stack)->top == NULL || (*stack)->bottom == NULL)
		return;
	if ((*stack)->top == (*stack)->bottom) {
		free((*stack)->bottom);
		(*stack)->top = NULL;
		(*stack)->bottom = NULL;
		return;
	}

	Node *curr = (*stack)->bottom;
	(*stack)->bottom = curr->next;
	free(curr);
}

void traverse(Stack **stack) {
	Node *curr = (*stack)->bottom;
	while (curr) {
		printf("%s\n", curr->data);
		printf("%lx\n", curr->offset);
		printf("%x\n", curr->hash);
		printf("%x\n", curr->data_size);
		printf("\n");
		curr = curr->next;
	}
}
