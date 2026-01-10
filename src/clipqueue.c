#include <clip_metadata.h>
#include <clipqueue.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ClipQueue *clipqueue_init() {
	ClipQueue *clipqueue = malloc(sizeof(ClipQueue));
	if (!clipqueue) return NULL;

	clipqueue->right = NULL;
	clipqueue->left = NULL;
	return clipqueue;
}

void clipqueue_deinit(ClipQueue **clipqueue) {
	Node *curr = (*clipqueue)->left;
	while (curr) {
		Node *temp = curr->next;
		free(curr->clip.data);
		free(curr);
		curr = temp;
	}
	free(*clipqueue);
	*clipqueue = NULL;
}

static inline Node *create_node(const ClipMetadata_t **clip) {
	Node *new_node = malloc(sizeof(Node));
	if (!new_node) return NULL;
	new_node->next = NULL;
	new_node->clip.offset = (*clip)->offset;
	new_node->clip.timestamp = (*clip)->timestamp;
	new_node->clip.hash = (*clip)->hash;
	new_node->clip.data_size = (*clip)->data_size;
	new_node->clip.new_clip = (*clip)->new_clip;
	new_node->clip.is_INCR = (*clip)->is_INCR;
	new_node->clip.data = NULL;

	if ((*clip)->data) {
		new_node->clip.data = malloc((*clip)->data_size + 1);
		if (!(new_node->clip.data)) {
			free(new_node);
			return NULL;
		}

		memcpy(new_node->clip.data, (*clip)->data, (*clip)->data_size);
		new_node->clip.data[(*clip)->data_size] = '\0';
	}

	return new_node;
}

void fill_clipqueue(ClipQueue **clipqueue, const ClipMetadata_t *clip) {
	if (!(*clipqueue)) return;

	Node *new_node = create_node(&clip);
	if (!new_node) return;

	if (!((*clipqueue)->left) && !((*clipqueue)->right)) {
		(*clipqueue)->right = new_node;
		(*clipqueue)->left = new_node;
		return;
	}

	if ((*clipqueue)->right->clip.timestamp <= clip->timestamp) {
		(*clipqueue)->right->next = new_node;
		(*clipqueue)->right = new_node;
		return;
	}

	Node *curr = (*clipqueue)->left;

	if (curr->clip.timestamp > clip->timestamp) {
		new_node->next = curr;
		(*clipqueue)->left = new_node;
		return;
	}

	while (curr && curr->next && curr->next->clip.timestamp < clip->timestamp) {
		curr = curr->next;
	}

	new_node->next = curr->next;
	curr->next = new_node;
}

void add(ClipQueue **clipqueue, const ClipMetadata_t *clip) {
	if (!(*clipqueue)) return;

	Node *new_node = create_node(&clip);
	if (!new_node) return;

	if (!((*clipqueue)->left) && !((*clipqueue)->right)) {
		(*clipqueue)->right = new_node;
		(*clipqueue)->left = new_node;
		return;
	}

	(*clipqueue)->right->next = new_node;
	(*clipqueue)->right = new_node;
}

void remove_left(ClipQueue **clipqueue) {
	if (!(*clipqueue) || !((*clipqueue)->left) || !((*clipqueue)->right))
		return;

	if ((*clipqueue)->right == (*clipqueue)->left) {
		free((*clipqueue)->left);
		(*clipqueue)->right = NULL;
		(*clipqueue)->left = NULL;
		return;
	}

	Node *curr = (*clipqueue)->left;
	(*clipqueue)->left = curr->next;
	free(curr->clip.data);
	free(curr);
}

void traverse(ClipQueue **clipqueue) {
	Node *curr = (*clipqueue)->left;
	while (curr) {
		printf("offset=%lx\n", curr->clip.offset);
		printf("timestamp=%ld\n", curr->clip.timestamp);
		printf("hash=%x\n", curr->clip.hash);
		printf("data_size=%lu\n", curr->clip.data_size);
		printf("is_INCR=%d\n", curr->clip.is_INCR);
		printf("new_clip=%d\n", curr->clip.new_clip);
		printf("\n");
		curr = curr->next;
	}
}
