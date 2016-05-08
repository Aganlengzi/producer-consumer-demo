/*
  实际上还是和只有一个缓冲item(本例相当于多个item)一样的.
  需要添加的是对于buffer的访问控制,同步有序访问
 */

/*
  生产者-消费者 基本模型
  一个生产者一个消费者 bounded buffer
  需要:
  1. 有界buffer 大小大于1
  2. buffer的锁
  3. 通知机制,producer写好之后通知consumer 
  4. flag 生产者和消费者之间的协议
  5. 一个生产者线程
  6. 一个消费者线程
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h> 

#define MAX_NUM 10
#define BUFFER_SZIE 2
//buffer数据类型 可扩展
typedef struct
{
	int p_index;
	int c_index;
	int num[BUFFER_SZIE];
}BUFFER;

//buffer
BUFFER buf;
//lock
pthread_mutex_t pc_mutex;
//通知
pthread_cond_t pc_condp, pc_condc;

/*
  生产者
 */

void * producer(void * nul)
{
	int i;
	for (i = 1; i <= MAX_NUM; ++i)
	{
		pthread_mutex_lock(&pc_mutex);
		
		//等待条件变量
		while(buf.p_index == buf.c_index)
		{
			printf("producer wait \n");
			pthread_cond_wait(&pc_condp, &pc_mutex);
		}
		//生产
		buf.num[buf.p_index] = i;
		printf("producer produces buf[%d] = %d \n", buf.p_index, buf.num[buf.p_index] );
		buf.p_index++;
		buf.p_index %= BUFFER_SZIE;
		sleep(2);
		//通知
		pthread_cond_signal(&pc_condc);
		pthread_mutex_unlock(&pc_mutex);
	}
	pthread_exit(NULL);
}

/*
  消费者
 */

void * consumer(void * nul)
{
	int i;
	for (i = 1; i <= MAX_NUM; ++i)
	{
		pthread_mutex_lock(&pc_mutex);
		
		//等待条件变量
		while(buf.p_index == (buf.c_index + 1)% BUFFER_SZIE) {
			printf("consumer wait \n");
	 		pthread_cond_wait(&pc_condc, &pc_mutex);
		}
		//条件达到
		buf.c_index += 1;
		buf.c_index %= BUFFER_SZIE;
		printf("consumer consumes buf[%d] = %d \n",buf.c_index, buf.num[buf.c_index]);
		buf.num[buf.c_index] = 0;
		sleep(1);
		//通知
		pthread_cond_signal(&pc_condp);
		pthread_mutex_unlock(&pc_mutex);
	}

	pthread_exit(NULL);
}



int main(int argc, char const *argv[])
{
	int i;
	pthread_t thread[3];
	pthread_attr_t attr;
	
	for (i = 0; i < BUFFER_SZIE; ++i)
	{
		buf.num[i] = 0;
	}
	buf.p_index = 1;
	buf.c_index = 0;

	//锁和条件变量
	pthread_mutex_init(&pc_mutex, NULL);
	pthread_cond_init(&pc_condp, NULL);
	pthread_cond_init(&pc_condc, NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	//producer
	pthread_create(&thread[0], &attr, producer, NULL);
	pthread_create(&thread[2], &attr, producer, NULL);
	//consumer
	pthread_create(&thread[1], &attr, consumer, NULL);

	//连接线程
	pthread_join(thread[0], NULL);
	pthread_join(thread[1], NULL);
	pthread_join(thread[2], NULL);

	//清理资源
	pthread_mutex_destroy(&pc_mutex);
	pthread_cond_destroy(&pc_condc);
	pthread_cond_destroy(&pc_condp);
	pthread_attr_destroy(&attr);

	pthread_exit(NULL);
	return 0;
}

