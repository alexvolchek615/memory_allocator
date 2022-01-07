CC=gcc
FLAGS=-g -Wall -Wpedantic

all:
	$(CC) $(FLAGS) main.c my_mem.c -o main