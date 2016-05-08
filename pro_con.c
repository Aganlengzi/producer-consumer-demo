/*
  生产者-消费者 基本模型
  一个生产者一个消费者一个buffer
  需要:
  1. 一个buffer
  2. buffer的锁
  3. 通知机制,producer写好之后通知consumer 
  4. flag 生产者和消费者之间的协议 本例中用的是BUFFER的num是否为0
  5. 一个生产者线程
  6. 一个消费者线程
 */

#include <pthread.h>
#include <stdio.h>

//buffer数据类型 可扩展
typedef struct
{
	int num;
}BUFFER;

#define MAX_NUM 1000

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
	for (i = 1; i < MAX_NUM; ++i)
	{
		pthread_mutex_lock(&pc_mutex);
		
		//等待条件变量
		while(buf.num != 0)
		{
			pthread_cond_wait(&pc_condp, &pc_mutex);
		}
		//生产
		buf.num = i;
		printf("producer produces %d \n", buf.num );
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
	for (i = 1; i < MAX_NUM; ++i)
	{
		pthread_mutex_lock(&pc_mutex);
		
		//等待条件变量
		while(buf.num == 0) {
	 		pthread_cond_wait(&pc_condc, &pc_mutex);
		}
		//条件达到
		printf("consumer consumes %d \n", buf.num);
		buf.num = 0;
		//通知consumer
		pthread_cond_signal(&pc_condp);
		pthread_mutex_unlock(&pc_mutex);
	}

	pthread_exit(NULL);
}



int main(int argc, char const *argv[])
{
	pthread_t thread[2];
	pthread_attr_t attr;
	
	buf.num = 0;

	//锁和条件变量
	pthread_mutex_init(&pc_mutex, NULL);
	pthread_cond_init(&pc_condp, NULL);
	pthread_cond_init(&pc_condc, NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	//producer
	pthread_create(&thread[0], &attr, producer, NULL);
	//consumer
	pthread_create(&thread[1], &attr, consumer, NULL);

	//连接线程
	pthread_join(thread[0], NULL);
	pthread_join(thread[1], NULL);

	//清理资源
	pthread_mutex_destroy(&pc_mutex);
	pthread_cond_destroy(&pc_condc);
	pthread_cond_destroy(&pc_condp);
	pthread_attr_destroy(&attr);

	pthread_exit(NULL);
	return 0;
}

/*
pthread_cond_wait的大致操作流程:
1. 解除已被调用线程锁住的锁
2. 等待条件,睡眠阻塞
3. 条件到来,醒来
4. 返回前锁住解开的调用线程锁住的锁
pthread_cond_signal用于唤醒在某个条件变量上等待的线程,一般是1个
pthread_cond_broadcast唤醒所有在某个条件变量上等待的线程

为什么要和mutex锁绑定使用?
the purppose of lock is to prevent simultaneous(同时的) request of wait()
if cond_signal happens, it will stop blocking and lock the mutex atomatically.

pthread_cond_signal即可以放在pthread_mutex_lock和pthread_mutex_unlock之间,
也可以放在pthread_mutex_lock和pthread_mutex_unlock之后，但是各有有缺点。

之间：
pthread_mutex_lock
    xxxxxxx
pthread_cond_signal
pthread_mutex_unlock

优点：下面那种方式的缺点
缺点：在某些线程的实现中，会造成等待线程(调用wait的线程)从内核中唤醒（由于cond_signal)然后又回到内核空间（因为cond_wait返回后会有原子加锁的行为），所以一来一回会有性能的问题。
但是在Linux Threads不会有这个问题，因为在Linux 线程中，有两个队列，分别是cond_wait队列和mutex_lock队列，
cond_signal只是让线程从cond_wait队列移到mutex_lock队列，而不用返回到用户空间，不会有性能的损耗。
所以在Linux中推荐使用这种模式。

之后：
pthread_mutex_lock
    xxxxxxx
pthread_mutex_unlock
pthread_cond_signal

优点：上面那种方式的缺点
缺点：如果unlock和signal之前，有个低优先级的线程(其它的)正在mutex上等待的话，
那么这个低优先级的线程就会抢占高优先级的线程（假设是wait这个cond的线程)，因为资源获得而获得执行,
而wait这个cond的线程资源还没有得到,只能等待. 典型的优先级翻转.

[1] http://blog.chinaunix.net/uid-27164517-id-3282242.html
*/