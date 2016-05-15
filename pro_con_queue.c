/*
  生产者消费者问题:
  在缓冲区不满时,一个或一组生产者(线程或进程)向缓冲区中插入数据,
  然后由一个或一组消费者(线程或进程)提取这些产品.
  下面是利用pthread实现的线程
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PRODUCER_NUM 3				//生产者数目
#define CONSUMER_NUM 2				//消费者数目
#define ITEM_NUM 5					//最多生产商品数
#define DELAY_TIME 3				//等待时间
#define QUEUE_SIZE (ITEM_NUM + 1)	//队列长度

typedef int ElemType;

/*
  head 指向队头前一个元素
  tail 指向队尾元素
  empty: head == tail
  full: (tial + 1)%QUEUE_SIZE == head
  insert: not full --> elem[++tail] = inserted_num
  delete: not empty --> delete[++head] 
 */
typedef struct 
{
	ElemType elem[QUEUE_SIZE];
	int head, tail;
}Queue;

Queue the_queue, *p_queue;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;

int ERROR_CODE;

/*
  func	: init the queue
  input	: Queue *p_queue
  output: none
 */
void init_queue(Queue *p_queue)
{
	memset(p_queue, 0, sizeof(*p_queue));
}

/*
  func	: judge if queue full
  input	: Queue* p_queue
  output: 0 not full
  	      1 is full
 */
int is_queue_full(Queue *p_queue)
{
	if(p_queue->head == (p_queue->tail + 1)% QUEUE_SIZE)
		return 1;
	else
		return 0;
}

/*
  func 	: judge if queue empty
  input : Queue* p_queue
  output: 0 not empty
  	      1 is empty
 */
int is_queue_empty(Queue *p_queue)
{
	if(p_queue->head == p_queue->tail)
		return 1;
	else
		return 0;
}

/*
  func 	: get total elem num
  input : Queue* p_queue
  output: num of elem
 */
int get_queue_num(Queue *p_queue)
{
	return (p_queue->tail + QUEUE_SIZE - p_queue->head)%QUEUE_SIZE;
}

/*
  func 	: insert an elem into the queue
  input : Queue* p_queue, inserted_elem
  output: 0 and ERROR_CODE=-1: full and wrong
  		  1: sucess
 */
int insert_queue(Queue *p_queue, ElemType inserted_elem)
{
	if(is_queue_full(p_queue))
	{
		printf("The queue is full\n");
		ERROR_CODE = -1;
		return 0;
	}
	else
	{
		p_queue->tail += 1;
		p_queue->tail %= QUEUE_SIZE;
		p_queue->elem[p_queue->tail] = inserted_elem;
		return 1;
	}
}

/*
  func 	: delete an elem from the queue
  input : Queue* p_queue
  output: 0 and ERROR_CODE = -1: empty and wrong  
  		  other: deleted elem 
 */
int delet_queue(Queue *p_queue)
{
	if(is_queue_empty(p_queue))
	{
		printf("The queue is empty\n");
		ERROR_CODE = -1;
		return 0;
	}
	else
	{
		p_queue->head += 1;
		p_queue->head %= QUEUE_SIZE;
		return p_queue->elem[p_queue->head]; 
	}
}

/*
  func 	: get tail of the queue
  input : Queue* p_queue
  output: the tail of the queue
 */
int get_queue_tail(Queue *p_queue)
{
	return p_queue->tail;
}

/*
  func 	: get head of the queue
  input : Queue* p_queue
  output: the head of the queue
 */
int get_queue_head(Queue *p_queue)
{
	return p_queue->head;
}

/*
  func 	: the consumer thread, detached and is a dead loop
  input : void *para, the index of the all_threads array convenient for locating
  output: void *
 */
void * consumer_thread(void *para)
{
	long thread_no = (long)para;
	while(1)
	{
		pthread_mutex_lock(&queue_lock);
		while(is_queue_empty(p_queue))
		{
			pthread_cond_wait(&consumer_cond, &queue_lock);
		}
		delet_queue(p_queue);
		if(get_queue_num(p_queue) == ITEM_NUM - 1)
		{
			pthread_cond_broadcast(&producer_cond);
		}
		printf("consumer thread[%ld] deletes queue[%d]=%d\n", thread_no, p_queue->head, p_queue->elem[p_queue->head]);
		pthread_mutex_unlock(&queue_lock);
		sleep(rand()%DELAY_TIME + 1);
	}
}


/*
  func 	: the producer thread, detached and is a dead loop
  input : void *para, the index of the all_threads array convenient for locating
  output: void *
*/
void * producer_thread(void *para)
{
	long thread_no = (long)para;
	int tmp;
	while(1)
	{
		pthread_mutex_lock(&queue_lock);
		while(is_queue_full(p_queue))
		{
			pthread_cond_wait(&producer_cond, &queue_lock);
		}
		tmp = get_queue_tail(p_queue);
		insert_queue(p_queue, tmp);
		if(get_queue_num(p_queue) == 1)
		{
			pthread_cond_broadcast(&consumer_cond);
		}
		printf("producer thread[%ld] produces queue[%d]=%d\n", thread_no, p_queue->tail, tmp );
		pthread_mutex_unlock(&queue_lock);
		sleep(rand()%DELAY_TIME + 1);
	}
}



int main(int argc, char const *argv[])
{
	// using pointer but not the variable
	p_queue = &the_queue;
	// storing all the ids of the threads
	pthread_t all_threads[CONSUMER_NUM + PRODUCER_NUM];
	// attributes
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	// tmp index
	int index;

	init_queue(p_queue);
	
	// creating the consumers
	for (index = 0; index < CONSUMER_NUM; ++index)
	{
		pthread_create(&all_threads[index], &attr, consumer_thread, (void*)index);
	}

	sleep(2);
	
	// creating the producers
	for (index = 0; index < PRODUCER_NUM; ++index)
	{
		pthread_create(&all_threads[index + CONSUMER_NUM], &attr, producer_thread, (void*)index);
	}

	//destroy the initialized attribute variable
	pthread_attr_destroy(&attr);

	//pthread_exit(NULL);
	while(1);
	return 0;
}


/*
  test the basic functions about the queue
 */
/*void QueueTest(Queue *p_queue)
{
	int i;
	int max_num_of_elem = 10;

	printf("The queue size is %d \n", get_queue_num(p_queue));
	printf("head = %d \n", p_queue->head );
	printf("tail = %d \n", p_queue->tail );
	for(i = 0; i < max_num_of_elem; i++)
	{
		insert_queue(p_queue, i);
		printf("The queue size is %d \n", get_queue_num(p_queue));
		printf("head = %d \n", p_queue->head );
		printf("tail = %d \n====\n", p_queue->tail );
	}

	printf("The queue size is %d \n", get_queue_num(p_queue));
	printf("head = %d \n", p_queue->head );
	printf("tail = %d \n", p_queue->tail );
	for(i = 0; i < max_num_of_elem-1; i++)
	{
		delet_queue(p_queue);
		printf("The queue size is %d \n", get_queue_num(p_queue));
		printf("head = %d \n", p_queue->head );
		printf("tail = %d \n====\n", p_queue->tail );
	}
}
*/



