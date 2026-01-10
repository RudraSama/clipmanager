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
	Node *left;
	Node *right;
} ClipQueue;

ClipQueue *clipqueue_init();

void clipqueue_deinit(ClipQueue **clipqueue);

void fill_clipqueue(ClipQueue **clipqueue, const ClipMetadata_t *clip);

void add(ClipQueue **clipqueue, const ClipMetadata_t *clip);

void remove_left(ClipQueue **clipqueue);

void traverse(ClipQueue **clipqueue);

#endif
