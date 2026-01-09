#include <clip_metadata.h>
#include <stack.h>
#include <stdbool.h>
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
		free(curr->clip.data);
		free(curr);
		curr = temp;
	}
	free(*stack);
}

void push(Stack **stack, const ClipMetadata_t *clip) {
	if (*stack == NULL) return;

	Node *new_node = malloc(sizeof(Node));
	if (!new_node) return;
	new_node->next = NULL;
	new_node->clip.offset = clip->offset;
	new_node->clip.timestamp = clip->timestamp;
	new_node->clip.hash = clip->hash;
	new_node->clip.data_size = clip->data_size;
	new_node->clip.new_clip = clip->new_clip;
	new_node->clip.is_INCR = clip->is_INCR;
	new_node->clip.data = NULL;

	// If no data, still push to stack for handling INCR
	if (clip->data != NULL) {
		new_node->clip.data = malloc(clip->data_size + 1);
		if (new_node->clip.data == NULL) {
			free(new_node);
			return;
		}

		memcpy(new_node->clip.data, clip->data, clip->data_size);
		new_node->clip.data[clip->data_size] = '\0';
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
		printf("offset=%lx\n", curr->clip.offset);
		printf("timestamp=%lld\n", curr->clip.timestamp);
		printf("hash=%x\n", curr->clip.hash);
		printf("data_size=%lu\n", curr->clip.data_size);
		printf("is_INCR=%d\n", curr->clip.is_INCR);
		printf("new_clip=%d\n", curr->clip.new_clip);
		printf("\n");
		curr = curr->next;
	}
}
