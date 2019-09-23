#ifndef _QUEUE_H
#define _QUEUE_H
#include "stdint.h"
#define   DEPTH      50
#pragma pack(2)
typedef struct {
	uint16_t value[125]; //用于传输数据
} item_t, *item_ptr;


#pragma pack()

#pragma pack(1)
typedef struct {
	uint8_t front;
	uint8_t rear;
	uint8_t count;
	item_t array[DEPTH];
} queue_t, *queue_ptr;
#pragma pack()
extern queue_t m_queue;

void queue_init(queue_t* Q);
int queue_empty(queue_t* Q);
int queue_enqueue(queue_t* Q, item_t* item);
int queue_dequeue(queue_t* Q, item_t* item);
#endif
