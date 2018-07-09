.PHONY: clean
CC = clang++
CFLAGS = -g -Wall -std=c++11

all: my_chat

%.o: %.cc my_chat.h
	$(CC) $(CFLAGS) -c -o $@ $<

my_chat: server.o handler.o my_chat.o safe_map.o
	$(CC) $(CFLAGS) -lpthread -ljsoncpp -o $@ $+

clean:
	rm -f *.o my_chat

