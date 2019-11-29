#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MASK 0x00FF
#define PAGE_SHIFT 7
#define BUFFER_SIZE 16
#define NUM_FRAMES 1024 / 2
#define FRAME_SIZE 128
#define NUM_PAGES 1024 / 2
#define PAGE_SIZE 128
#define TLB_SIZE 128

typedef struct
{
  int page_number;
  int frame_number;
} page_frame_node;

void init();
int checkTLB(int page_number);
void addToTLB(int page_number, int frame_number);
int getFrameNumberByPageNumber(int page_number);
void addToPageTable(int page_number, int frame_number);

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
clock_t start;
double TLB_timer;

int main(int argc, char *argv[])
{

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

    start = clock();
    int frame_number = checkTLB(page_number);
    TLB_timer = (double)(clock() - start) / CLOCKS_PER_SEC;

    if (frame_number == -1)
    {

      frame_number = getFrameNumberByPageNumber(page_number);

      if (frame_number == -1)
      {

        frame_number = frame_counter++ % NUM_PAGES;

        addToPageTable(page_number, frame_number);

        faults++;
      }

      addToTLB(page_number, frame_number);
    }
    else
    {

      TLB_hits++;
    }

    int physical_address = frame_number << 8;
    physical_address = physical_address | offset;

    read_value = physical_memory[frame_number][offset];

    printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, read_value);
  }

  double p = (double)(operation_counter - faults) / (double)operation_counter;
  double h = (double)TLB_hits / (double)operation_counter;
  double t = TLB_timer;
  printf("Total de enderecos referenciados = %d\n", operation_counter);
  printf("Total de paginas referencidas: %d\n", operation_counter);
  printf("Total de referencias as paginas que resultaram em acertos: %d\n",
         operation_counter - faults);
  printf("Total de referencias as paginas que resultaram em falhas: %d\n",
         faults);
  printf("Total de operacoes E/S: %d\n", faults);
  printf("Tempo de acesso efetivo = %f ns\n",
         h * (t + 200) + (1 - h) * ((1 - p) * 8000000 + p * 200 + t));
  printf("TLB Hits = %d\n", TLB_hits);
  printf("TLB Hit Rate = %f\n", (double)TLB_hits / (double)operation_counter);
  fclose(address_file);

  return 0;
}

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

int checkTLB(int page_number)
{
  int i = 0;

  for (; i < TLB_SIZE; i++)
  {
    if (TLB[i].page_number == page_number)
      return TLB[i].frame_number;
  }

  return -1;
}

void addToTLB(int page_number, int frame_number)
{
  TLB[TLB_counter].page_number = page_number;
  TLB[TLB_counter].frame_number = frame_number;

  TLB_counter = (TLB_counter + 1) % TLB_SIZE;
}

int getFrameNumberByPageNumber(int page_number)
{
  int i = 0;

  for (; i < NUM_PAGES; i++)
  {
    if (page_table[i].page_number == page_number)
      return page_table[i].frame_number;
  }

  return -1;
}

void addToPageTable(int page_number, int frame_number)
{
  page_table[frame_number].page_number = page_number;
  page_table[frame_number].frame_number = frame_number;
}