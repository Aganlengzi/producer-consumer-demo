CFLAGS = -lpthread -pthread -lm
CC = gcc


all:
	$(CC) $(CFLAGS) pro_con.c -o pro_con

pro_con_db:
	$(CC) $(CFLAGS) pro_con_db.c -o pro_con_db

clean:
	rm -rf *.o pro_con pro_con_db