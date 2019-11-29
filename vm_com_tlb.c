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

typedef struct
{
  int page_number;
  int frame_number;
} page_frame_node;

void init();
int check_TLB(int page_number);
void add_to_TLB(int page_number, int frame_number);
int check_page_table(int page_number);
void add_to_page_table(int page_number, int frame_number);

FILE *address_file;

signed char physical_memory[NUM_FRAMES][FRAME_SIZE];
page_frame_node page_table[NUM_PAGES];
page_frame_node TLB[TLB_SIZE];

char address[BUFFER_SIZE];
int logical_address;
signed char read_value;

int operation_counter = 0;
int frame_counter = 0;
int faults = 0;
int TLB_hits = 0;
int TLB_counter = 0;
int memory_access_counter = 0;

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: ./vm [input file]\n");
    return -1;
  }

  address_file = fopen(argv[1], "r");

  if (address_file == NULL)
  {
    fprintf(stderr, "Error opening %s\n", argv[1]);
    return -1;
  }

  init();

  while (fgets(address, BUFFER_SIZE, address_file) != 0)
  {
    operation_counter++;
    logical_address = atoi(address);

    int page_number = logical_address >> PAGE_SHIFT;
    page_number = page_number & MASK;
    int offset = logical_address & MASK;

    int frame_number = check_TLB(page_number);

    if (frame_number == -1)
    {
      frame_number = check_page_table(page_number);
      memory_access_counter++;

      if (frame_number == -1)
      {
        frame_number = frame_counter++ % NUM_PAGES;
        add_to_page_table(page_number, frame_number);
        faults++;
      }
      add_to_TLB(page_number, frame_number);
    }
    else
    {
      TLB_hits++;
    }

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
  printf("Acessos ao TLB = %d\n", TLB_hits);
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

  for (int i = 0; i < TLB_SIZE; i++)
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
