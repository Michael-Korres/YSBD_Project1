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
	BF_Block_Init(&first_block);					//CREATE A BLOCK => THE SPECIAL BLOCK OF AN EMPTY HEAP FILE

	BF_AllocateBlock(file_describer,first_block);	//BLOCK => THE SPECIAL BLOCK(1st) OF AN EMPTY HEAP FILE
	

	// HP_block_info * curr_HP_block_info = malloc(sizeof(HP_block_info));	//INITIALIZE THE BLOCK INFO
	// curr_HP_block_info->next_block_address = NULL;
	// // curr_HP_block_info->next_block_id = 
	// curr_HP_block_info->records_num = 0;
	// curr_HP_block_info->capacity = BF_BLOCK_SIZE;
	// curr_HP_block_info->occupied_mem = occupied_mem;
	

	void * data = BF_Block_GetData(first_block);	//GET ACCESS TO BLOCK'S DATA SECTION
	HP_info * curr_HP_info = data ;	//ASSIGN THE ADDRESS TO A HP_info POINTER TO HANDLE AND STORE
									//THE INFO EASIER

	
	curr_HP_info->filename = fileName;				//CONSTANT
	curr_HP_info->file_describer = file_describer;	//MODIFIED IN EACH BF_OpenFile()
	curr_HP_info->is_heap_file = 1;					//CONSTANT
	curr_HP_info->last_block_id = 0;				//MODIFIED (probably) IN HP_InsertEntry()
	curr_HP_info->last_block_address = NULL;		//MODIFIED (probably) IN HP_InsertEntry()
	curr_HP_info->block_capacity_in_bytes = BF_BLOCK_SIZE;	//CONSTANT
	

	BF_Block_SetDirty(first_block);					//SET DIRTY AS MODIFIED
	BF_UnpinBlock(first_block);						//UNPIN BLOCK AS WE DO NOT NEED IT
	

	CALL_BF(BF_CloseFile(file_describer))			//CLOSE FILE
	CALL_BF(BF_Close());							//CLOSE BUFFER => SAVE IN THE DISK
	return 0;
}

HP_info* HP_OpenFile(char *fileName){	
	if (BF_Init(LRU) != BF_OK)return NULL;	//INITIALIZE BUFFER

	int file_describer;
	if (BF_OpenFile(fileName, &file_describer) != BF_OK)return NULL;	//OPEN FILES

	BF_Block * first_block;	
	BF_Block_Init(&first_block);	//ALLOCATE A BLOCK TO EXCTRACT THE SPECIAL(1st) BLOCK FROM THE FILE

	if (BF_GetBlock(file_describer,0,first_block) != BF_OK)return NULL;	//ASSIGN 1st BLOCK

	HP_info * curr_HP_info;	//DEFINE HP_info POINTER
	void * data = BF_Block_GetData(first_block);	//EXTRACT BLOCK'S DATA A.K.A SPECIAL INFO
	curr_HP_info = (HP_info *)data;			//STORE THE SPECIAL INFO IN THE HP_Info POINTER

	curr_HP_info->file_describer = file_describer;
	BF_Block_SetDirty(first_block);
	BF_UnpinBlock(first_block);	

	return curr_HP_info;

}


int HP_CloseFile(HP_info* HP_info ){


	CALL_BF(BF_CloseFile(HP_info->file_describer));	//CLOSE FILE
	CALL_BF(BF_Close());							//CLOSE BUFFER => SAVE IN THE DISK
	return 0;

}

int HP_InsertEntry(HP_info *HP_info, Record record){
	


	return 0;
}

int HP_GetAllEntries(HP_info *HP_info, int value)
{
	return 0;
}


void HP_infoPrint(HP_info * curr_HP_info){
printf("curr_HP_info->filename: %s\n",curr_HP_info->filename);
printf("curr_HP_info->last_block_id: %d\n",curr_HP_info->last_block_id);
printf("curr_HP_info->file_describer: %d\n",curr_HP_info->file_describer);
printf("curr_HP_info->is_heap_file: %d\n",curr_HP_info->is_heap_file);	
}