#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "sht_table.h"
#include "ht_table.h"
#include "record.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

char *name_to_id[100][20];
int maxy, maxi;
char *filename_ht;

int SHT_CreateSecondaryIndex(char *sfileName, int buckets, char *fileName)
{
  int fd1, fd2;
  filename_ht = fileName;
  if (buckets == 0)
  {
    printf("why you use the hash table.\n");
  }

  if (BF_CreateFile(sfileName) != BF_OK)
  {
    printf("cannot create file\n");
    return -1;
  }

  if (BF_OpenFile(sfileName, &fd1) != BF_OK)
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
  hp_block.name[0] = 'S';
  hp_block.name[1] = 'H';
  hp_block.buckets = buckets;
  if ((blocksNeeded * sizeof(int)) > BF_BLOCK_SIZE - 8)
  {
    return -1;
  }
  // int this[blocksNeeded];
  memcpy(data, &hp_block, sizeof(HT_block_info));
  for (int i = 0; i < blocksNeeded; i++)
  {
    int this = 0;
    memcpy(data + sizeof(HT_block_info) + i * sizeof(int), &this, sizeof(int));
  }

  // memcpy(data + sizeof(HT_block_info), &this, (blocksNeeded * sizeof(int)));

  BF_Block_SetDirty(block);
  if (BF_UnpinBlock(block) != BF_OK)
  {
    return -1;
  }
  // Write "SHT" in the beginning of first block to signify

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

SHT_info *SHT_OpenSecondaryIndex(char *indexName)
{

  // Open file.
  int fd1, count;
  if (BF_OpenFile(indexName, &fd1) != BF_OK)
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
  SHT_block_info ht;

  memcpy(&ht, data, sizeof(HT_block_info));

  // Check if it's not SHT file and close it.

  if (ht.name[0] != 'S' && ht.name[1] != 'H') //(ht->name[0] != 'H' && ht->name[1] != 'T')
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
  int a = (BF_BUFFER_SIZE / ht.buckets);

  int b = (BF_BUFFER_SIZE / a);

  int blocksNeeded = (b * a);
  maxi = blocksNeeded;
  maxy = (BF_BLOCK_SIZE) / sizeof(Record);
  SHT_info *curr_HP_info = malloc(sizeof(SHT_info));
  curr_HP_info->Sfilename = indexName;
  curr_HP_info->fd1 = fd1;
  curr_HP_info->HTfilename = filename_ht;
  curr_HP_info->buckets = ht.buckets; // ht->blocks;
  curr_HP_info->max_recod_in_block = (BF_BLOCK_SIZE) / sizeof(Record);
  curr_HP_info->max_blocks = count;
  curr_HP_info->sizeofblocks = count - 1;
  // Close block.

  return curr_HP_info;
}

int SHT_CloseSecondaryIndex(SHT_info *SHT_info)
{
  if (BF_CloseFile(SHT_info->fd1) != BF_OK)
  {
    return -1;
  }

  return 0;
}

int SHT_SecondaryInsertEntry(SHT_info *sht_info, Record record, int block_id)
{
  int fd1 = sht_info->fd1;
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

  int *curr_number = (int *)(data_for_first + sizeof(HT_block_info) + block_id * sizeof(int));
  memcpy(&ht, data_for_first, sizeof(HT_block_info));

  if (curr_number[0] < sht_info->max_recod_in_block)
  {

    if (BF_GetBlock(fd1, block_id, Block_to_insert) != BF_OK)
    {
      return -1;
    }
    char *data = BF_Block_GetData(Block_to_insert);
    int data_memory = curr_number[0] * sizeof(Record);
    memcpy(data + data_memory, &record, sizeof(Record));
    name_to_id[block_id][curr_number[0]] = record.name;
    printf("test1,%d,%d,%s\n", block_id, curr_number[0], name_to_id[block_id][curr_number[0]]);
    BF_Block_SetDirty(Block_to_insert);
    if (BF_UnpinBlock(Block_to_insert) != BF_OK)
    {
      return -1;
    }
    BF_Block_Destroy(&Block_to_insert);
  }
  curr_number[0]++;
  int da = curr_number[0];
  memcpy(data_for_first + 8 + block_id * 4, &da, 4);
  BF_Block_SetDirty(Block_to_see);
  BF_Block_Destroy(&Block_to_see);
  return block_id;
}

int SHT_SecondaryGetAllEntries(HT_info *ht_info, SHT_info *sht_info, char *name)
{
  int printed = 0, totalBlocks;
  int fd1 = sht_info->fd1;
  int buckets = sht_info->buckets;
  BF_Block *Block_to_insert, *Block_to_see;
  BF_Block_Init(&Block_to_insert);
  BF_Block_Init(&Block_to_see);
  char *data_for_first;

  if (BF_GetBlock(fd1, 0, Block_to_see) != BF_OK)
  {
    return -1;
  }

  if (BF_GetBlockCounter(fd1, &totalBlocks) != BF_OK)
  {
    return -1;
  }

  data_for_first = BF_Block_GetData(Block_to_see);
  HT_block_info ht;
  memcpy(&ht, data_for_first, sizeof(HT_block_info));

  if (name == NULL)
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
  {

    for (int i = 1; i < maxi + 1; i++)
    {
      int *curr_number = (int *)(data_for_first + sizeof(HT_block_info) + i * sizeof(int));
      // printf("%d ,%d ,%s\n", i, curr_number[0], name_to_id[i][curr_number[0]]);
      for (int j = 0; j < curr_number[0]; j++)
      {
        printf("test6,%d,%d,%s ,%s\n", i, j, name_to_id[i][j], name);
        if (name_to_id[i][j] == name)
        {

          BF_GetBlock(fd1, i, Block_to_insert);
          char *data = BF_Block_GetData(Block_to_insert);
          Record rec;

          memcpy(&rec, data + j * sizeof(Record), sizeof(Record));
          printRecord(rec);
          printed++;

          if (BF_UnpinBlock(Block_to_insert) != BF_OK)
          {
            return -1;
          }
        }
      }
    }
  }
  return printed;
}
