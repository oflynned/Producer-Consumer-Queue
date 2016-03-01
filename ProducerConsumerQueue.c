#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define true 1
#define false 0

#define NUM_PRODUCERS 4
#define NUM_CONSUMERS 2
#define MAX_QUEUE_CAPACITY 8
#define MAX_TRANSACTIONS 256

pthread_t consumers[NUM_CONSUMERS];
pthread_t producers[NUM_PRODUCERS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_empty = PTHREAD_COND_INITIALIZER;

int to_produce = 0;
int is_done = 0;

typedef struct Queue
{
	int capacity;
	int size;
	int front;
	int rear;
	int* elements;
} Queue;

Queue* create_queue(int max_elements);
void destroy_queue(Queue* Q);
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

int main(int argc, char* argv[])
{
	Queue* Q = create_queue(MAX_QUEUE_CAPACITY);

	//initialise threads for consuming and producing
	int i;
	for(i=0; i<NUM_PRODUCERS; i++)
	{
		printf("Created producer thread %d\n", i);
		pthread_create(&producers[i], NULL, produce, (void*) Q);
	}
	for(i=0; i<NUM_CONSUMERS; i++)
	{
		printf("Created consumer thread %d\n", i);
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
	
	destroy_queue(Q);
	
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

void destroy_queue(Queue* Q)
{
	free(Q);
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
	//consume items in the queue while the queue isn't empty, else wait for empty signal
	Queue* Q = (Queue*) arg;
	while(is_done == 0)
	{
		//usleep(250000);
		pthread_mutex_trylock(&mutex);
		if(Q->size == Q->capacity)
		{
			printf("Queue is full, item being consumed...\n");
			pthread_mutex_unlock(&mutex);
			pthread_cond_broadcast(&queue_full);
			dequeue(Q);
		}
		else if(Q->size < Q->capacity && !is_empty(Q))
		{
			printf("Queue is not full, item being consumed...\n");
			pthread_mutex_unlock(&mutex);
			pthread_cond_broadcast(&queue_full);
			dequeue(Q);
		}
		else
		{
			printf("Queue is empty, consumer waiting...\n");
			pthread_mutex_unlock(&mutex);
			pthread_cond_wait(&queue_empty, &mutex);
			printf("Queue is no longer empty, obtaining item...\n");
		}
		print_queue(Q);
		printf("**current transaction**: %d\n", to_produce);
	}
	return NULL;
}

void* produce(void* arg)
{
	//produce and add to queue while under capacity, else broadcast wait signal
	Queue* Q = (Queue*) arg;
	while(to_produce < MAX_TRANSACTIONS)
	{
		//usleep(250000);
		pthread_mutex_trylock(&mutex);
		if(Q->size == 0)
		{
			printf("Queue is empty, item being added...\n");
			to_produce++;
			enqueue(Q, generate_random_number());
			pthread_mutex_unlock(&mutex);
			pthread_cond_broadcast(&queue_empty);
		}
		else if(Q->size>0 & !is_full(Q))
		{
			printf("Queue is not full, item being added...\n");
			to_produce++;
			enqueue(Q, generate_random_number());
			pthread_mutex_unlock(&mutex);
			pthread_cond_broadcast(&queue_empty);
		}
		else
		{
			printf("Queue is full, producer is waiting...\n");
			pthread_mutex_unlock(&mutex);
			pthread_cond_wait(&queue_full, &mutex);
			printf("Queue is no longer full, producer is back to work...\n");
		}
		
		if(is_empty(Q) && to_produce >= MAX_TRANSACTIONS)
		{
			printf("\n*****is done!******\n");
			is_done = 1;
		}
		print_queue(Q);
	}
	return NULL;
}

