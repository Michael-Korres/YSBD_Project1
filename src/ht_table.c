#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"

int open_files = 0;

// Extremely sophisticated hash function:
int hash(int id, int buckets)
{
  return id % buckets;
}

int HT_CreateFile(char *fileName, int buckets)
{
  // Create and open HT file.
  if (buckets == 0)
  {
    printf("why you use the hash table.\n");
  }
  int fd1;

  if (BF_CreateFile(fileName) != BF_OK)
  {
    printf("cannot create file\n");
    return -1;
  }

  if (BF_OpenFile(fileName, &fd1) != BF_OK)
  {
    printf("cannot open file\n");
    return -1;
  }

  // Create first block.
  BF_Block *block;
  BF_Block_Init(&block);

  if (BF_AllocateBlock(fd1, block) != BF_OK)
  {
    return -1;
  }

  char *data = BF_Block_GetData(block);

  // Calculate blocks needed for index.

  int a = (BF_BUFFER_SIZE / buckets);

  int b = (BF_BUFFER_SIZE / a);

  int blocksNeeded = (b * a); ////oysiastika ama exoyme bucket=6 tote BF_BUFFER_SIZE % buckets=16 ,etsi 6*16=96 ara max 96 block gia na einai isa

  HT_block_info hp_block;
  hp_block.name[0] = 'H';
  hp_block.name[1] = 'T';
  hp_block.buckets = buckets;
  if ((blocksNeeded * sizeof(int)) > BF_BLOCK_SIZE - 8)
  {
    return -1;
  }
  // int this[blocksNeeded];
  memcpy(data, &hp_block, sizeof(HT_block_info));
  for (int i = 0; i < blocksNeeded; i++)
  {
    int how_many_records_inside_the_block_i = 0;
    memcpy(data + sizeof(HT_block_info) + i * sizeof(int), &how_many_records_inside_the_block_i, sizeof(int));
  }

  BF_Block_SetDirty(block);
  if (BF_UnpinBlock(block) != BF_OK)
  {
    return -1;
  }
  // Write "HT" in the beginning of first block to signify

  if (BF_AllocateBlock(fd1, block) != BF_OK)
  {
    return -1;
  }

  if (blocksNeeded > BF_BUFFER_SIZE)
  {
    printf("the bucket is to big for the max size of buffer.\n");
    return -1;
  }
  // Allocate blocks for index.
  for (int i = 0; i < blocksNeeded; i++)
  {
    if (BF_AllocateBlock(fd1, block) != BF_OK)
    {
      return -1;
    }
    if (BF_UnpinBlock(block) != BF_OK)
    {
      return -1;
    }
  }

  BF_Block_Destroy(&block);
  if (BF_CloseFile(fd1) != BF_OK)
  {
    return -1;
  }

  return 0;
}

HT_info *HT_OpenFile(char *fileName)
{
  // If we have reached max open files, return error.
  if (open_files >= BF_MAX_OPEN_FILES)
  {
    fprintf(stderr, "Can't open more files.\n");
    return NULL;
  }

  open_files++;

  // Open file.
  int fd1, count;
  if (BF_OpenFile(fileName, &fd1) != BF_OK)
  {
    fprintf(stderr, "bfopenfile.\n");
    return NULL;
  }

  BF_GetBlockCounter(fd1, &count);
  // Init block.
  BF_Block *block;
  BF_Block_Init(&block);
  char *data;

  if (BF_GetBlock(fd1, 0, block) != BF_OK)
  {
    fprintf(stderr, "bfgetblock.\n");
    return NULL;
  }
  data = BF_Block_GetData(block);
  // HT_block_info * hp_block=malloc(sizeof(HT_block_info));
  HT_block_info ht;

  memcpy(&ht, data, sizeof(HT_block_info));

  // Check if it's not HT file and close it.

  if (ht.name[0] != 'H' && ht.name[1] != 'T') //(ht->name[0] != 'H' && ht->name[1] != 'T')
  {
    if (BF_UnpinBlock(block) != BF_OK)
    {
      fprintf(stderr, "bfunpinblock.\n");
      return NULL;
    }
    BF_Block_Destroy(&block);
    if (BF_CloseFile(fd1) != BF_OK)
    {
      fprintf(stderr, "bfclose.\n");
      return NULL;
    }
    return NULL;
  }

  // GIVE HP_INFO
  HT_info *curr_HT_info = malloc(sizeof(HT_info));
  curr_HT_info->filename = fileName;
  curr_HT_info->fd1 = fd1;
  curr_HT_info->buckets = ht.buckets; // ht->blocks;
  curr_HT_info->max_recod_in_block = (BF_BLOCK_SIZE) / sizeof(Record);
  curr_HT_info->max_blocks = count;
  curr_HT_info->sizeofblocks = count - 1;
  // Close block.
  if (BF_UnpinBlock(block) != BF_OK)
  {
    printf("cannot unpin block\n");
    return NULL;
  }
  BF_Block_Destroy(&block);
  return curr_HT_info;
}

int HT_CloseFile(HT_info *HT_info)
{
  // Find the index in the table of indexes and close the desired file.
  if (BF_CloseFile(HT_info->fd1) != BF_OK)
  {
    return -1;
  }
  open_files--;
  return 0;
}

int HT_InsertEntry(HT_info *ht_info, Record record)
{

  // Get hashed value:
  int fd1 = ht_info->fd1;
  // giati xtipas edo
  int buckets = ht_info->buckets;

  int hashvalue = hash(record.id, buckets);

  int block_number = hashvalue * (BF_BUFFER_SIZE / (BF_BUFFER_SIZE / buckets));
  // Find bucket:
  // printf("block=%d,hash=%d,buff/bucket=%d.buf=%d\n", block_number, hashvalue, (BF_BUFFER_SIZE / buckets), BF_BUFFER_SIZE);
  int bucket;

  BF_Block *Block_to_insert, *Block_to_see;
  BF_Block_Init(&Block_to_insert); // BAZO TO RECORD EDO
  BF_Block_Init(&Block_to_see);    // PLIROFORIES
  char *data_for_first;

  if (BF_GetBlock(fd1, 0, Block_to_see) != BF_OK)
  {
    return -1;
  }

  data_for_first = BF_Block_GetData(Block_to_see);

  HT_block_info ht;

  int *curr_number = (int *)(data_for_first + sizeof(HT_block_info) + block_number * sizeof(int));
  memcpy(&ht, data_for_first, sizeof(HT_block_info));
  // memcpy(curr_number, data_for_first + sizeof(HT_block_info) + block_number * sizeof(int), sizeof(int));
  // printf("test4 ,%d,%ld \n", curr_number[0], sizeof(HT_block_info) + block_number * sizeof(int));

  // elenxo posa record exei to block
  if (block_number == 0)
  {
    block_number++;
  }
  while (curr_number[0] >= ht_info->max_recod_in_block)
  {
    block_number++;
    if (block_number == ht_info->max_blocks)
    {
      printf("it has go to the last hash and the last hash is full\n");
      return -1;
    }
    memcpy(&curr_number, data_for_first + 8 + block_number * 4, 4);
  }

  if (curr_number[0] < ht_info->max_recod_in_block)
  {

    if (BF_GetBlock(fd1, block_number, Block_to_insert) != BF_OK)
    {
      return -1;
    }
    char *data = BF_Block_GetData(Block_to_insert);
    int data_memory = curr_number[0] * sizeof(Record);
    memcpy(data + data_memory, &record, sizeof(Record));
    BF_Block_SetDirty(Block_to_insert);
    if (BF_UnpinBlock(Block_to_insert) != BF_OK)
    {
      return -1;
    }
    BF_Block_Destroy(&Block_to_insert);
  }
  curr_number[0]++;
  int da = curr_number[0];
  memcpy(data_for_first + 8 + block_number * 4, &da, 4);
  BF_Block_SetDirty(Block_to_see);
  BF_Block_Destroy(&Block_to_see);
  return block_number;
}

int HT_GetAllEntries(HT_info *ht_info, void *value)
{

  /// check this shit;
  int fd1 = ht_info->fd1;
  int buckets = ht_info->buckets;
  int id = *(int *)value;
  int hashvalue = hash(id, buckets);
  int block_number = (BF_BUFFER_SIZE / (BF_BUFFER_SIZE / buckets)) * hashvalue;
  // Calculate blocks in index.

  int printed = 0;

  BF_Block *Block_to_insert, *Block_to_see;
  BF_Block_Init(&Block_to_insert);
  BF_Block_Init(&Block_to_see);
  char *data_for_first;
  if (BF_GetBlock(fd1, 0, Block_to_see) != BF_OK)
  {
    return -1;
  }
  data_for_first = BF_Block_GetData(Block_to_see);
  HT_block_info ht;
  memcpy(&ht, data_for_first, sizeof(HT_block_info));

  int totalBlocks;

  if (BF_GetBlockCounter(fd1, &totalBlocks) != BF_OK)
  {
    return -1;
  }
  // AN value is NULL, print ola ta records sto hash file
  if (value == NULL)
  {

    int block_number = 1;

    while (block_number < totalBlocks)
    {
      int *curr_number = (int *)(data_for_first + sizeof(HT_block_info) + block_number * sizeof(int));
      BF_GetBlock(fd1, block_number, Block_to_insert);
      char *data = BF_Block_GetData(Block_to_insert);
      for (int i = 0; i < curr_number[0]; i++)
      {
        Record rec;
        memcpy(&rec, data + i * sizeof(Record), sizeof(Record));
        printRecord(rec);
        printed++;
      }
      if (BF_UnpinBlock(Block_to_insert) != BF_OK)
      {
        return -1;
      }
      block_number++;
    }
  }
  else
  { // AN den einai tipose osa exoyn id=value

    if (block_number == 0)
    {
      block_number++;
    }

    while (block_number < totalBlocks)
    {

      int *curr_number = (int *)(data_for_first + sizeof(HT_block_info) + block_number * sizeof(int));
      BF_GetBlock(fd1, block_number, Block_to_insert);
      char *data = BF_Block_GetData(Block_to_insert);
      int check = 0, var = 0;
      for (int i = 0; i < curr_number[0]; i++)
      {
        Record rec;
        memcpy(&rec, data + i * sizeof(Record), sizeof(Record));

        if (hash(rec.id, buckets) == id)
        {

          check = 1;
          printRecord(rec);
          printed++;
        }
      }

      if (BF_UnpinBlock(Block_to_insert) != BF_OK)
      {
        return -1;
      }

      block_number++;
    }
  }
  if (BF_UnpinBlock(Block_to_see) != BF_OK)
  {
    return -1;
  }
  BF_Block_Destroy(&Block_to_see);
  BF_Block_Destroy(&Block_to_insert);
  return printed;
}
