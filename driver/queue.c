#include "std.h"
#include "queue.h"

queue_t m_queue;

void queue_init(queue_t* Q)
{
	memset(Q, 0, sizeof(queue_t));
}

int queue_empty(queue_t* Q)
{
	if (Q->count == 0 && Q->front == Q->rear) {
		return 1;
	}
	return 0;
}

int queue_enqueue(queue_t* Q, item_t* item)
{
	if (Q->count == DEPTH && Q->front == Q->rear) {
		return 0;
	}
	Q->array[Q->rear] = *item;
	Q->count++;
	Q->rear = (Q->rear + 1) % DEPTH;
	return 1;
}

int queue_dequeue(queue_t* Q, item_t* item)
{
	if (queue_empty(Q)) {
		return 0;
	}
	*item = Q->array[Q->front];
	Q->count--;
	Q->front = (Q->front + 1) % DEPTH;
	return 1;
}
