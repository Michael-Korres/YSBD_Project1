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
int hash(int ID, int buckets)
{
  return ID % buckets;
}

int HT_CreateFile(char *fileName, int buckets)
{
  // Create and open HT file.
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
  memcpy(data, hp_block, 10);
  // Write number of buckets after "HT" string.
  // Save changes to first block.

  BF_Block_SetDirty(block);

  if (BF_UnpinBlock(block) != BF_OK)
  {
    return -1;
  }

  // Calculate blocks needed for index.
  int integersInABlock = BF_BLOCK_SIZE / sizeof(int);
  int blocksNeeded = buckets / integersInABlock + 1;
  if (blocksNeeded > BF_BUFFER_SIZE)
  {
    printf("the bucket is to big for the max size of buffer");
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
  int fd1;
  if (BF_OpenFile(fileName, &fd1) != BF_OK)
  {
    return NULL;
  }

  // Init block.
  BF_Block *block;
  BF_Block_Init(&block);
  void *data;

  if (BF_GetBlock(fd1, 0, block) != BF_OK)
  {
    return NULL;
  }
  data = BF_Block_GetData(block);
  HT_block_info *ht;
  ht = data + BF_BUFFER_SIZE - sizeof(HT_block_info);
  // Check if it's not HT file and close it.
  if (ht->name != 'HT')
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
  curr_HP_info->buckets = ht->buckets;
  curr_HP_info->max_recod_in_block = (BF_BLOCK_SIZE - 2 * sizeof(char) - 2 * sizeof(int)) % sizeof(Record);
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
  int hashValue = hash(record.id, buckets);
  // Find bucket:
  int integersInABlock = BF_BLOCK_SIZE / sizeof(int);
  int blockToGoTo = hashValue / integersInABlock + 1;
  int positionInBlock = hashValue % integersInABlock;
  int bucket;
  BF_Block *Block_to_insert;
  BF_Block *recordBlock;
  BF_Block_Init(&recordBlock);
  BF_Block_Init(&Block_to_insert);

  void *data;
  if (BF_GetBlock(fd1, blockToGoTo, Block_to_insert) != BF_OK)
  {
    return -1;
  }
  data = BF_Block_GetData(Block_to_insert);
  HT_block_info *ht;
  ht = data + BF_BUFFER_SIZE - sizeof(HT_block_info);

  if (bucket == 0)
  {
  }
  else
  { // Bucket already exists:
  }
  BF_Block_Destroy(&Block_to_insert);
  return 0;
}

int HT_GetAllEntries(HT_info *ht_info, void *value)
{
  int fd1 = ht_info->fd1;
  int buckets = ht_info->buckets;

  // Calculate blocks in index.
  int integersInABlock = BF_BLOCK_SIZE / sizeof(int);
  int blocksInIndex = buckets / integersInABlock + 1;
  int printed = 0;
  BF_Block *recordBlock;
  BF_Block_Init(&recordBlock);

  int totalBlocks;
  if (BF_GetBlockCounter(fd1, &totalBlocks) != BF_OK)
  {
    return -1;
  }

  // If value is NULL, print every record in the hash file
  if (value == NULL)
  {
  }
  else
  { // Otherwise find the record in the hash table using the same operations as in InsertEntry
  }
  return printed;
}