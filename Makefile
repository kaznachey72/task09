CC = gcc
FLAGS = -Wall -Wextra -Wpedantic -std=c11 -D_GNU_SOURCE
INIH_DIR = ./third_party/inih

ex09: main.o app_control.o server.o logger.o config.o libini.a
	$(CC) $^ -L. -lini -lpthread -o $@

main.o: main.c
	$(CC) $(FLAGS) -c $^ -o $@

app_control.o: app_control.c
	$(CC) $(FLAGS) -c $^ -o $@

server.o: server.c
	$(CC) $(FLAGS) -c $^ -o $@

logger.o: logger.c
	$(CC) $(FLAGS) -c $^ -o $@

config.o: config.c 
	$(CC) $(FLAGS) -I$(INIH_DIR) -c $^ -o $@

libini.a: $(INIH_DIR)/ini.c
	$(CC) -I$(INIH_DIR) -c $^ -o ini.o
	ar rcs $@ ini.o 

clean:
	rm -f ex09 *.o *.log *.a *.ini
