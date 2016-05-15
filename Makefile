CFLAGS = -lpthread -pthread -lm
CC = gcc


all:
	$(CC) $(CFLAGS) pro_con.c -o pro_con

pro_con_db:
	$(CC) $(CFLAGS) pro_con_db.c -o pro_con_db

pro_con_queue:
	$(CC) $(CFLAGS) pro_con_queue.c -o pro_con_queue

clean:
	rm -rf *.o pro_con pro_con_db pro_con_queue