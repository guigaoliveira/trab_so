/**
 * This is a demonstration of virtual memory for 
 * when the backing store is twice the size as memory.
 *
 * It uses FIFO algorithm to manage page faults.
 *
 * @Author Brandon Denning
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MASK 0x00FF
#define PAGE_SHIFT 8
#define BUFFER_SIZE 10
#define TLB_SIZE 128
#define NUM_FRAMES 64
#define FRAME_SIZE 64
#define NUM_PAGES 64
#define PAGE_SIZE 64

//structs
typedef struct
{
  int page_number;
  int frame_number;
} page_frame_node;

//prototypes
void init();
int check_TLB(int page_number);
void add_to_TLB(int page_number, int frame_number);
int check_page_table(int page_number);
void add_to_page_table(int page_number, int frame_number);
void print_TLB();
void print_page_table();

//globals

//files
FILE *address_file;

//arrays
signed char physical_memory[NUM_FRAMES][FRAME_SIZE];
page_frame_node page_table[NUM_PAGES];
page_frame_node TLB[TLB_SIZE];

//reading variables
char address[BUFFER_SIZE];
int logical_address;
signed char read_value;

//counters
int operation_counter = 0;
int frame_counter = 0;
int faults = 0;
int TLB_hits = 0;
int TLB_counter = 0;
int memory_access_counter = 0;

int main(int argc, char *argv[])
{
  //assert that there are 2 arguments before beginning execution
  if (argc != 2)
  {
    fprintf(stderr, "Usage: ./vm [input file]\n");
    return -1;
  }

  //open file addresses with third argument
  address_file = fopen(argv[1], "r");

  //handle null file case
  if (address_file == NULL)
  {
    fprintf(stderr, "Error opening %s\n", argv[1]);
    return -1;
  }

  //initialize all arrays: memory, page table, TLB
  init();

  //read each number from address file
  while (fgets(address, BUFFER_SIZE, address_file) != 0)
  {
    //increment total operations
    operation_counter++;

    //convert read address to an int
    logical_address = atoi(address);

    //get the page number and offset from the logical address
    int page_number = logical_address >> PAGE_SHIFT;
    page_number = page_number & MASK;
    int offset = logical_address & MASK;
    //check the TLB for the page number
    int frame_number = check_TLB(page_number);

    //if the page number isn't found in the TLB
    if (frame_number == -1)
    {
      //check the page table for the page number
      frame_number = check_page_table(page_number);
      memory_access_counter++;
      //if the page number isn't found in the page table
      if (frame_number == -1)
      {
        //create a frame number from a counter
        frame_number = frame_counter++ % NUM_PAGES;
        //add a map node to the page table
        add_to_page_table(page_number, frame_number);
        //increment number of page faults
        faults++;
      }

      //if there was a TLB miss, add it to the TLB
      add_to_TLB(page_number, frame_number);
    }
    else
    {
      //the page was in the TLB
      TLB_hits++;
    }

    //get the physical address from the frame number
    int physical_address = frame_number << 8;
    physical_address = physical_address | offset;

    //get the requested value
    read_value = physical_memory[frame_number][offset];
    memory_access_counter++;
    // printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, read_value);
  }

  //print summary
  printf("Total de enderecos referenciados = %d\n", operation_counter);
  printf("Total de paginas referencidas: %d", operation_counter);
  printf("Total de referencias as paginas que resultaram em acertos: %d\n", operation_counter - faults);
  printf("Total de referencias as paginas que resultaram em falhas: %d\n", faults);
  printf("Taxa de falhas de pagina = %f\n", (double)faults / (double)operation_counter);
  printf("TLB Hits = %d\n", TLB_hits);
  printf("TLB Hit Rate = %f\n", (double)TLB_hits / (double)operation_counter);
  printf("Total de operacoes E/S: %d\n", faults);

  fclose(address_file);

  return 0;
}

/*
 * initializes the memory table, page table and TLB
 */
void init()
{
  int i = 0;
  int j = 0;

  for (i = 0; i < NUM_FRAMES; i++)
  {
    for (j = 0; j < FRAME_SIZE; j++)
      physical_memory[i][j] = 0;
  }

  for (i = 0; i < NUM_PAGES; i++)
  {
    page_table[i].page_number = -1;
    page_table[i].frame_number = -1;
  }

  for (i = 0; i < TLB_SIZE; i++)
  {
    TLB[i].page_number = -1;
    TLB[i].frame_number = -1;
  }
}

/*
 * determine if the specified page number is in the TLB
 */
int check_TLB(int page_number)
{
  int i = 0;

  for (; i < TLB_SIZE; i++)
  {
    if (TLB[i].page_number == page_number)
      return TLB[i].frame_number;
  }

  return -1;
}

/*
 * add a map node to the TLB
 */
void add_to_TLB(int page_number, int frame_number)
{
  TLB[TLB_counter].page_number = page_number;
  TLB[TLB_counter].frame_number = frame_number;

  TLB_counter = (TLB_counter + 1) % TLB_SIZE;
}

/*
 * get the frame at the given page_number; returns -1 if it is not found
 */
int check_page_table(int page_number)
{
  for (int i = 0; i < NUM_PAGES; i++)
  {
    if (page_table[i].page_number == page_number)
      return page_table[i].frame_number;
  }

  return -1;
}

/*
 * add the map node to the page table
 */
void add_to_page_table(int page_number, int frame_number)
{
  page_table[frame_number].page_number = page_number;
  page_table[frame_number].frame_number = frame_number;
}

/*
 * prints the contents of the TLB
 */
void print_TLB()
{

  printf("\nTLB\n");

  for (int i = 0; i < TLB_SIZE; i++)
  {
    printf("%d | %d\n", TLB[i].page_number, TLB[i].frame_number);
  }

  printf("\n");
}

/*
 * prints the contents of the page table
 */
void print_page_table()
{
  int i = 0;

  printf("\nPage Table\n");

  for (; i < NUM_PAGES; i++)
  {
    printf("%d | %d\n", page_table[i].page_number, page_table[i].frame_number);
  }

  printf("\n");
}
