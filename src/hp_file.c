#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return HP_ERROR;        \
    }                         \
  }

int HP_CreateFile(char *fileName)
{

  if (BF_Init(LRU) != BF_OK)
  {
    return -1;
  }
  if (BF_CreateFile(fileName) != BF_OK)
  {
    return -1;
  }
  return 0;
}

HP_info *HP_OpenFile(char *fileName)
{
  // HP_INFO COUNT OF BLOCK IF CODE IS SLOW BF_GetBlockCounter(fd1, &blocks_num)
  int fd1, blocks_num;
  BF_Block *block;
  HP_info *HP = malloc(sizeof(HP_info));
  HP->name = fileName;

  if (BF_OpenFile(fileName, &fd1) != BF_OK)
  {
    return NULL;
  }
  if (blocks_num == 0)
  {
    HP->bytes_in_block = 0;
    HP->block_num = 0;
    return HP;
  }
  BF_GetBlockCounter(fd1, &blocks_num);
  HP->fd1 = fd1;
  BF_GetBlock(fd1, blocks_num, block);
  HP->bytes_in_block = sizeof(BF_Block_GetData(block));
  HP->block_num = blocks_num;
  return HP; /// se
}

int HP_CloseFile(HP_info *hp_info)
{
  if (BF_CloseFile(hp_info->fd1) != BF_OK)
  {
    return -1;
  }
  if (BF_Close() != BF_OK)
  {
    return -1;
  }

  return 0;
}

int HP_InsertEntry(HP_info *hp_info, Record record)
{
  return 0;
}

int HP_GetAllEntries(HP_info *hp_info, int value)
{
  return 0;
}
