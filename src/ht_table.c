#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"

#define OFFSET 2 * sizeof(int)

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
    return -1;
  }

  if (BF_OpenFile(fileName, &fd1) != BF_OK)
  {
    return -1;
  }
  // Create first block.
  BF_Block *block;
  BF_Block_Init(&block);

  if (BF_AllocateBlock(fd1, block) != BF_OK)
  {
    return -1;
  }
  void *data = BF_Block_GetData(block);

  // Write "HT" in the beginning of first block to signify
  // we have an HT file.

  HT_block_info hp_block;
  hp_block.name[0] = 'H';
  hp_block.name[1] = 'T';
  hp_block.buckets = buckets;
  hp_block.curr_number = 0;
  memcpy(data, &hp_block, 10);
  // Write number of buckets after "HT" string.
  // Save changes to first block.

  BF_Block_SetDirty(block);

  if (BF_UnpinBlock(block) != BF_OK)
  {
    return -1;
  }

  // Calculate blocks needed for index.
  int blocksNeeded = (BF_BUFFER_SIZE % (BF_BUFFER_SIZE % buckets)) * (BF_BUFFER_SIZE % buckets); ////oysiastika ama exoyme bucket=6 tote BF_BUFFER_SIZE % buckets=16 ,etsi 6*16=96 ara max 96 block gia na einai isa
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

  // Make changes and close HT file.
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
    return NULL;
  }
  BF_GetBlockCounter(fd1, &count);
  // Init block.
  BF_Block *block;
  BF_Block_Init(&block);
  void *data;

  if (BF_GetBlock(fd1, 0, block) != BF_OK)
  {
    return NULL;
  }
  data = BF_Block_GetData(block);
  HT_block_info ht;
  memcpy(&ht, data, 10);
  // Check if it's not HT file and close it.
  if (ht.name[0] != 'H' && ht.name[1] != 'T')
  {
    if (BF_UnpinBlock(block) != BF_OK)
    {
      return NULL;
    }
    BF_Block_Destroy(&block);
    if (BF_CloseFile(fd1) != BF_OK)
    {
      return NULL;
    }
    return NULL;
  }

  // GIVE HP_INFO
  HT_info *curr_HP_info = malloc(sizeof(HT_info));
  curr_HP_info->filename = fileName;
  curr_HP_info->fd1 = fd1;
  curr_HP_info->buckets = ht.buckets;
  curr_HP_info->max_recod_in_block = (BF_BLOCK_SIZE - sizeof(HT_block_info)) % sizeof(Record);
  curr_HP_info->max_blocks = count;
  // Close block.
  if (BF_UnpinBlock(block) != BF_OK)
  {
    return NULL;
  }
  BF_Block_Destroy(&block);

  return curr_HP_info;
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
  int buckets = ht_info->buckets;
  int hashvalue = hash(record.id, buckets);
  int block_number = hashvalue * (BF_BUFFER_SIZE % (BF_BUFFER_SIZE % buckets));
  // Find bucket:
  int bucket;
  BF_Block *Block_to_insert;
  BF_Block_Init(&Block_to_insert);

  void *data;
  if (BF_GetBlock(fd1, block_number, Block_to_insert) != BF_OK)
  {
    return -1;
  }
  data = BF_Block_GetData(Block_to_insert);
  HT_block_info ht;
  memcpy(&ht, data, 10);
  if (ht.curr_number < ht_info->max_recod_in_block) ////an exei xoro to proto block toy bucket valto ekei
  {
    int data_memory = ht.curr_number * sizeof(Record) + sizeof(HT_block_info) + 1;
    memcpy(data + data_memory, &record, sizeof(Record));
  }
  else
  { // allios des ta epomena block toy bucket kai ama xreiastei ektos aftoy
    while (ht.curr_number >= ht_info->max_recod_in_block)
    {
      if (BF_UnpinBlock(Block_to_insert) != BF_OK)
      {
        return -1;
      }
      block_number++;
      if (block_number == ht_info->max_blocks + 1)
      {
        printf("the hash table its full.\n");
        return -1;
      }
      if (BF_GetBlock(fd1, block_number, Block_to_insert) != BF_OK)
      {
        return -1;
      }
      data = BF_Block_GetData(Block_to_insert);
      memcpy(&ht, data, 10);
    }
    if (ht.curr_number < ht_info->max_recod_in_block)
    {
      int data_memory = ht.curr_number * sizeof(Record) + sizeof(HT_block_info) + 1;
      memcpy(data + data_memory, &record, sizeof(Record));
    }
  }
  ht.curr_number++;
  memcpy(data, &ht, sizeof(HT_block_info));
  BF_Block_SetDirty(Block_to_insert);
  BF_Block_Destroy(&Block_to_insert);
  return block_number;
}

int HT_GetAllEntries(HT_info *ht_info, void *value)
{
  int fd1 = ht_info->fd1;
  int buckets = ht_info->buckets;

  // Calculate blocks in index.

  int printed = 0;
  int id = *(int *)value;
  int hashvalue = hash(id, buckets);
  BF_Block *Block_to_insert;
  BF_Block_Init(&Block_to_insert);
  int totalBlocks, block_number = 0;

  if (BF_GetBlockCounter(fd1, &totalBlocks) != BF_OK)
  {
    return -1;
  }
  // AN value is NULL, print ola ta records sto hash file
  if (value == NULL)
  {
    HT_block_info ht;
    while (block_number <= totalBlocks)
    {
      BF_GetBlock(fd1, block_number, Block_to_insert);
      void *data = BF_Block_GetData(Block_to_insert);
      memcpy(&ht, data, sizeof(HT_block_info));
      for (int i = 0; i < ht.curr_number; i++)
      {
        Record rec;
        memcpy(&rec, data + sizeof(HT_block_info) + i * sizeof(Record), sizeof(Record));
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
    block_number = (BF_BUFFER_SIZE % (BF_BUFFER_SIZE % buckets)) * hashvalue;
    HT_block_info ht;
    while (block_number <= totalBlocks)
    {
      BF_GetBlock(fd1, block_number, Block_to_insert);
      void *data = BF_Block_GetData(Block_to_insert);
      memcpy(&ht, data, sizeof(HT_block_info));
      int check = 0, var = 0;
      for (int i = 0; i < ht.curr_number; i++)
      {
        Record rec;
        memcpy(&rec, data + sizeof(HT_block_info) + i * sizeof(Record), sizeof(Record));
        if (rec.id == id)
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
      if (check == 0) // afto poy kano einai oysiastika blepo an se dyo sinexomena blocks den exei value=record.id ara den exei kata 99% balei ekei kapoio record ama exei mpei einai kaki hash table
      {
        if (var == 1)
        {
          BF_Block_Destroy(&Block_to_insert);
          return printed;
        }
        var = 1;
      }
      else
      {
        var = 0;
      } ///////////////gia id meta apo ena simio den tha exei
      block_number++;
    }
  }

  BF_Block_Destroy(&Block_to_insert);
  return printed;
}
