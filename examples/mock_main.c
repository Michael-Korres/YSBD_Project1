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
  // curr_HP_info->capacity_of_records = curr_HP_info->block_capacity_in_bytes...


  // HP_infoPrint(info);



  HP_CloseFile(curr_HP_info);
  BF_Close();
}
