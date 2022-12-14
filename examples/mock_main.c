#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"

#define RECORDS_NUM 1000 // you can change it if you want
#define FILE_NAME "data.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {
  BF_Init(LRU);

  HP_CreateFile(FILE_NAME);
  HP_info* curr_HP_info = HP_OpenFile(FILE_NAME);


  // HP_info_Print(curr_HP_info);

  Record rec = randomRecord();

  HP_InsertEntry(curr_HP_info,rec);

  /////////////////////////

  //CALL_OR_DIE(BF_Init(LRU));
  BF_Block *block;
  BF_Block_Init(&block);

  
  
  int blocks_num;
  int fd1 = curr_HP_info->file_describer;
  CALL_OR_DIE(BF_GetBlockCounter(fd1, &blocks_num));

  void* data;

  for (int i = 1; i < blocks_num; ++i) {
    printf("Contents of Block %d\n\t",i);
    CALL_OR_DIE(BF_GetBlock(fd1, i, block));
    data = BF_Block_GetData(block);
    Record* rec= data;
    printRecord(rec[0]);
    // printf("\t");
    // printRecord(rec[1]);
    CALL_OR_DIE(BF_UnpinBlock(block));
  }

  BF_Block_Destroy(&block);


  //////////////////////

  HP_CloseFile(curr_HP_info);
  BF_Close();
}
