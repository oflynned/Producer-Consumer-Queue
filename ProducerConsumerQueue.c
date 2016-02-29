#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 4

typedef struct Queue
{
	int capacity;
	int size;
	int front;
	int rear;
	int *elements;
} Queue;

Queue* create_queue(int max_elements);
void enqueue(Queue* Q, int element);
void dequeue(Queue* Q);
int front(Queue* Q);

int main(int argc, char* argv[])
{
	Queue* Q = create_queue(5);
	int i;
	for(i=0; i<5; i++)
	{
		enqueue(Q, i);
		printf("%d is now queued\n", i);
	}
	
	printf("Frontmost element is %d\n", front(Q));
	
	return 0;
}

//queue data structure
Queue* create_queue(int max_elements)
{
	Queue* Q = (Queue*) malloc(sizeof(Queue));
	Q->elements = (int*) malloc(sizeof((int*)max_elements));
	Q->size = 0;
	Q->capacity = max_elements;
	Q->front = 0;
	Q->rear = -1;
	return Q;
}

void enqueue(Queue* Q, int element)
{
	if(Q->size == Q->capacity)
	{
		printf("Queue has reached capacity of %d elements\n", Q->capacity);
	}
	else
	{
		//not empty, possible to add another n - size elements
		Q->size++;
		Q->rear++;
		//append 0 to rear if not at capacity -- prevents overflow
		if(Q->rear == Q->capacity)
		{
			Q->rear = 0;
		}
		Q->elements[Q->rear] = element;
	}
}

void dequeue(Queue* Q)
{
	//prevent underflow, remove nth element from queue
	if(Q->size == 0)
	{
		printf("Queue now empty with size %d", Q->size);
	}
	else
	{
		Q->size--;
		Q->front++;
		if(Q->front == Q->capacity)
		{
			Q->front = 0;
		}
	}
}

int front(Queue* Q)
{
	//return the frontmost element of the queue
	if(Q->size==0)
	{
		printf("Queue empty, nothing is at the front\n");
		return 0;
	}
	else
	{
		return Q->elements[Q->front];
	}
}

//buffer


//queue manipulation
void consume(){}
void produce(){}
