CC	= g++
CFLAGS	= -g -std=c++11 -O3 -Wall -Wextra 

all: main

main: main.o cache.o lru.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c -o main.o main.cpp

cache.o: cache.cpp
	$(CC) $(CFLAGS) -c -o cache.o cache.cpp

lru.o: lru.cpp
	$(CC) $(CFLAGS) -c -o lru.o lru.cpp
	