# Makefile for virtual memory program
CC=gcc
CFLAGS=-Wall

all: vm_com_tlb vm_sem_tlb

vm_com_tlb: vm_com_tlb.o
	$(CC) $(CFLAGS) -o vm_com_tlb vm_com_tlb.o

vm_sem_tlb: vm_sem_tlb.o
	$(CC) $(CFLAGS) -o vm_sem_tlb vm_sem_tlb.o