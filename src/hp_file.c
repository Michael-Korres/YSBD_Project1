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
	CALL_BF(BF_OpenFile(fileName,&file_describer))

	BF_Block *first_block;
	BF_Block_Init(&first_block);					//CREATE A BLOCK


	int file_block_count;
	BF_GetBlockCounter(file_describer,&file_block_count);
	printf("Block num is: %d\n",file_block_count);

	BF_GetBlockCounter(file_describer,&file_block_count);
	printf("Block num is: %d\n",file_block_count);

	BF_GetBlockCounter(file_describer,&file_block_count);
	printf("Block num is: %d\n",file_block_count);



	BF_AllocateBlock(file_describer,first_block);	//ALLOCATE IT AS 1st OF THE HEAP FILE
	

	// HP_block_info * curr_HP_block_info = malloc(sizeof(HP_block_info));	//INITIALIZE THE BLOCK INFO
	// curr_HP_block_info->next_block_address = NULL;
	// // curr_HP_block_info->next_block_id = 
	// curr_HP_block_info->records_num = 0;
	// curr_HP_block_info->capacity = BF_BLOCK_SIZE;
	// curr_HP_block_info->capacity = occupied_mem;
	
	void * data = BF_Block_GetData(first_block);	//GET ACCESS TO BLOCK'S DATA SECTION

	// int* block_id = (int*)data;
	// *block_id = 0;

	HP_info * curr_HP_info = data ;//+ sizeof(int); //= malloc(sizeof(HP_info)); //INITIALIZE HEAP FILE INFO
	
	
	curr_HP_info->filename = fileName;
	curr_HP_info->file_describer = 100;
	curr_HP_info->is_heap_file = 1;
	curr_HP_info->last_block_id = 0;
	curr_HP_info->last_block_address = NULL;

	BF_Block_SetDirty(first_block);					//SET DIRTY AS MODIFIED
	BF_UnpinBlock(first_block);						//UNPIN BLOCK AS WE DO NOT NEED IT
	
	// HP_infoPrint((HP_info*)data);

	BF_GetBlockCounter(file_describer,&file_block_count);

	printf("Block num is: %d\n",file_block_count);

	
	
	
	CALL_BF(BF_CloseFile(file_describer))
	CALL_BF(BF_Close());
	return 0;
}

HP_info* HP_OpenFile(char *fileName){	
	if (BF_Init(LRU) != BF_OK)return NULL;	

	int file_describer;
	if (BF_OpenFile(fileName, &file_describer) != BF_OK)return NULL;

	BF_Block * first_block;	
	BF_Block_Init(&first_block);

	// int file_block_count;
	// BF_GetBlockCounter(file_describer,&file_block_count);

	// printf("Block num is: %d\n",file_block_count);


	if (BF_GetBlock(file_describer,0,first_block) != BF_OK)return NULL;

	HP_info * curr_HP_info;// = malloc(sizeof(HP_info)); //INITIALIZE HEAP FILE INFO
	void * data = BF_Block_GetData(first_block);
	curr_HP_info = (HP_info *)data;
	

	HP_infoPrint(curr_HP_info);

	return curr_HP_info;



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


void HP_infoPrint(HP_info * curr_HP_info){
printf("curr_HP_info->filename: %s\n",curr_HP_info->filename);
printf("curr_HP_info->last_block_id: %d\n",curr_HP_info->last_block_id);
printf("curr_HP_info->file_describer: %d\n",curr_HP_info->file_describer);
printf("curr_HP_info->is_heap_file: %d\n",curr_HP_info->is_heap_file);	
}