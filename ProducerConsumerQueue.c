#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 4
#define MAX_QUEUE_CAPACITY 128
#define BUFFER_CAPACITY 2048
#define UNITS_PER_CONSUMER 64
#define MAX_TRANSACTIONS 1024

pthread_t consumers[NUM_CONSUMERS];
pthread_t producers[NUM_PRODUCERS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_empty = PTHREAD_COND_INITIALIZER;

int to_produce = 0;
int to_consume = MAX_TRANSACTIONS;

typedef struct Queue
{
	int capacity;
	int size;
	int front;
	int rear;
	int* elements;
} Queue;

Queue* create_queue(int max_elements);
void enqueue(Queue* Q, int element);
void dequeue(Queue* Q);
int front(Queue* Q);
int end(Queue* Q);
bool is_full(Queue* Q);
bool is_empty(Queue* Q);
void print_queue(Queue* Q);

int generate_random_number();

void* consume(void* arg);
void* produce(void* arg);
void signal(Queue* Q);

int main(int argc, char* argv[])
{
	Queue* Q = create_queue(MAX_QUEUE_CAPACITY);

	//initialise threads for consuming and producing
	int i;
	for(i=0; i<NUM_PRODUCERS; i++)
	{
		pthread_create(&producers[i], NULL, produce, (void*) Q);
	}
	for(i=0; i<NUM_CONSUMERS; i++)
	{
		pthread_create(&consumers[i], NULL, consume, (void*) Q);
	}
	
	//join threads
	for(i=0; i<NUM_PRODUCERS; i++)
	{
		pthread_join(producers[i], NULL);
	}
	for(i=0; i<NUM_CONSUMERS; i++)
	{
		pthread_join(consumers[i], NULL);
	}
	
	return 0;
}

//queue data structure
Queue* create_queue(int max_elements)
{
	Queue* Q = (Queue*) malloc(sizeof(Queue));
	Q->elements = (int*) malloc(sizeof(max_elements));
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
		Q->rear = Q->rear+1;
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

int end(Queue* Q)
{
	//return the endmost element of the queue
	if(Q->size>Q->capacity)
	{
		printf("Queue full, cannot add more\n");
		return 0;
	}
	else
	{
		return Q->elements[Q->rear];
	}
}

bool is_full(Queue* Q)
{
	return Q->size == Q->capacity ? true : false;
}

bool is_empty(Queue* Q)
{
	return Q->size == 0 ? true : false;
}

void print_queue(Queue* Q)
{
	printf("\n--------------\n\n");
	if(Q->size != 0)
	{
		int i;
		for(i=0; i < Q->size; i++)
		{
			if(i == 0)
			{
				printf("Queue: ");
			}
		
			if(i != Q->size - 1)
			{
				printf("%d, ", Q->elements[i]);
			}
			else
			{
				printf("%d.", Q->elements[i]);
			}
		}
	}
	else
	{
		printf("Queue empty!");
	}
	
	printf("\n\n--------------\n\n");
}

//queue manipulation
int generate_random_number()
{
	return (lrand48() % 100 + 1);
}

void* consume(void* arg)
{
	Queue* Q = (Queue*) arg;
	while(to_consume > 0)
	{
		pthread_mutex_lock(&mutex);
		if(Q->size == 0)
		{
			printf("Queue is empty, consumer waiting...\n");
			pthread_mutex_unlock(&mutex);
			pthread_cond_broadcast(&buffer_empty);
			printf("Queue is no longer empty, obtaining item...\n");
		}
		else if(Q->size > 0 && Q->size < Q->capacity)
		{
			printf("Queue is not empty, item being consumed...\n");
			pthread_mutex_unlock(&mutex);
			pthread_cond_broadcast(&buffer_not_full);
			to_consume--;
			dequeue(Q);
		}
		else
		{
			printf("Queue is full, item being consumed...\n");
			pthread_mutex_unlock(&mutex);
			pthread_cond_broadcast(&buffer_not_full);
			to_consume--;
			dequeue(Q);
		}
		print_queue(Q);
	}
}

void* produce(void* arg)
{
	//produce and add to queue while under capacity, else broadcast wait signal
	Queue* Q = (Queue*) arg;
	while(to_produce < MAX_TRANSACTIONS)
	{
		sleep(1);
		pthread_mutex_lock(&mutex);
		if(Q->size == Q->capacity)
		{
			printf("Queue is full, producer is waiting...\n");
			pthread_mutex_unlock(&mutex);
			pthread_cond_wait(&buffer_full, &mutex);
		}
		else if(Q->size < Q->capacity)
		{
			printf("Item added to queue by producer...\n");
			to_produce++;
			enqueue(Q, generate_random_number());
			pthread_mutex_unlock(&mutex);
			pthread_cond_broadcast(&buffer_not_full);
		}
		else
		{	
			printf("Queue is empty, item being added...\n");
			to_produce++;
			enqueue(Q, generate_random_number());
			pthread_mutex_unlock(&mutex);
			pthread_cond_wait(&buffer_not_full, &mutex);
		}
		print_queue(Q);
	}
}

