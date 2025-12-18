#include <stack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void push(Stack **stack, const char *data, unsigned long data_size) {
	if (*stack == NULL) return;

	Node *new_node = malloc(sizeof(Node));
	if (!new_node) return;
	new_node->next = NULL;
	new_node->data = strdup(data);

	if (!new_node->data) {
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
		curr = curr->next;
	}
}
