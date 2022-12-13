#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName){
    if (BF_Init(LRU) != BF_OK){
    return -1;
  }
  if (BF_CreateFile(fileName) != BF_OK)
  {
    return -1;
  }
  return 0;
}

HP_info* HP_OpenFile(char *fileName){
  // HP_INFO COUNT OF BLOCK IF CODE IS SLOW BF_GetBlockCounter(file_describer, &blocks_num)
  int file_describer;
  HP_info* HP = malloc(sizeof(HP_info));
  HP->filename = fileName;
  if (BF_OpenFile(fileName, &file_describer) != BF_OK)
  {
    return NULL;
  }
  int blocks_num;

  BF_GetBlockCounter(file_describer,&blocks_num);
  
  HP->last_block = blocks_num - 1;

  printf("The blocks_num: %d\n",blocks_num);

  HP->file_describer = file_describer;
  printf("The file describer: %d\n",file_describer);


  BF_Block *first_block;
  BF_Block_Init(&first_block);
  
  for(int i = 0; i < blocks_num;i++){  
    BF_GetBlock(file_describer,0,first_block);
    char* data_p = BF_Block_GetData(first_block);
    printf("Block: %d's data: %s\n",i,data_p);

  }
  
////////////////////////////////
  return HP;
}


int HP_CloseFile( HP_info* hp_info ){
   if (BF_CloseFile(hp_info->file_describer) != BF_OK)
  {
    return -1;
  }
  if (BF_Close() != BF_OK)
  {
    return -1;
  }

  return 0;
}

int HP_InsertEntry(HP_info* hp_info, Record record){
    return 0;
}

int HP_GetAllEntries(HP_info* hp_info, int value){
   return 0;
}

