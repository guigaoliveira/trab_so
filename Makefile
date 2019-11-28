# Makefile for virtual memory program
CC=gcc
CFLAGS=-Wall

all: vm vm-fifo vm-lru

vm: vm.o
	$(CC) $(CFLAGS) -o vm vm.o

vm-fifo: vm-fifo.o
	$(CC) $(CFLAGS) -o vm-fifo vm-fifo.o

vm-lru: vm-lru.o
	$(CC) $(CFLAGS) -o vm-lru vm-lru.o
