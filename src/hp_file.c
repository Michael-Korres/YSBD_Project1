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

int HP_CreateFile(char *fileName){
	CALL_BF(BF_CreateFile(fileName))			
	int file_describer;								//CREATE AND OPEN FILE
	CALL_BF(BF_OpenFile(fileName,file_describer))

	BF_Block *first_block;
	CALL_BF(BF_Init(first_block))					//CREATE A BLOCK

	BF_AllocateBlock(file_describer,first_block);	//ALLOCATE IT AS 1st OF THE HEAP FILE
	
	// HP_block_info * curr_HP_block_info = malloc(sizeof(HP_block_info));	//INITIALIZE THE BLOCK INFO
	// curr_HP_block_info->next_block_address = NULL;
	// // curr_HP_block_info->next_block_id = 
	// curr_HP_block_info->records_num = 0;
	// curr_HP_block_info->capacity = BF_BLOCK_SIZE;
	// curr_HP_block_info->capacity = occupied_mem;
	


	HP_info * curr_HP_info = malloc(sizeof(HP_info)); //INITIALIZE HEAP FILE INFO
	curr_HP_info->filename = fileName;
	curr_HP_info->file_describer = file_describer;
	curr_HP_info->is_heap_file = 1;
	curr_HP_info->last_block_id = 0;
	curr_HP_info->last_block_address = NULL;

	void * data = BF_Block_GetData(first_block);	//GET ACCESS TO BLOCK'S DATA SECTION
	memcpy(curr_HP_info,data,sizeof(HP_info));		//SAVE THE SPECIAL INFO
	BF_Block_SetDirty(first_block);					//SET DIRTY AS MODIFIED
	BF_UnpinBlock(first_block);						//UNPIN BLOCK AS WE DO NOT NEED IT

	free(curr_HP_info);
	CALL_BF(BF_CloseFile(file_describer))
	return 0;
}

HP_info* HP_OpenFile(char *fileName){
// 	int file_describer;
// 	if (BF_OpenFile(fileName, &file_describer) != BF_OK)return NULL;
// 	BF_Block * first_block;



// 	HP->file_describer = file_describer;  //Initializing HP_info->file_describer
// 	printf("The file describer: %d\n",file_describer);
	
// 	int blocks_num;

// 	BF_GetBlockCounter(file_describer,&blocks_num);
	
// 	HP->last_block_id = blocks_num - 1;  //Initializing HP_info->last_block_id

// 	printf("The blocks_num: %d\n",blocks_num);



// 	BF_Block *first_block;
// 	BF_Block_Init(&first_block);
	
// 	for(int i = 0; i < blocks_num;i++){  
// 		BF_GetBlock(file_describer,0,first_block);
// 		void * data = BF_Block_GetData(first_block);
// 		Record * record = data;
// 		printf("Block: %d's| City: %s\n",i,record->city);

// 	}
	
// ////////////////////////////////
// 	return HP;
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

int HP_InsertEntry(HP_info *hp_info, Record record)
{
	return 0;
}

int HP_GetAllEntries(HP_info *hp_info, int value)
{
	return 0;
}
