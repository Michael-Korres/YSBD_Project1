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
	


	void * data = BF_Block_GetData(first_block);	//GET ACCESS TO BLOCK'S DATA SECTION
	HP_info * curr_HP_info = data ;	//ASSIGN THE ADDRESS TO A HP_info POINTER TO HANDLE AND STORE
									//THE INFO EASIER

	
	curr_HP_info->filename = fileName;				//CONSTANT                                	
	curr_HP_info->file_describer = file_describer;	//MODIFIED IN EACH BF_OpenFile()          
	curr_HP_info->is_heap_file = 1;					//CONSTANT                               	
	curr_HP_info->last_block_address = NULL;		//MODIFIED (probably) IN HP_InsertEntry()
	curr_HP_info->block_capacity_of_records = 
		(BF_BLOCK_SIZE - sizeof(HP_block_info))/sizeof(Record);	//CONSTANT: under_bound((B - H)/r)
																//B:Block size,H: Header size and r:record size
	
	curr_HP_info->storage_location = first_block;	//WE HOLD THE BLOCK WERE THE HP_info IS STORED AND WE WILL DEALLOCATE THE RESPECTABLE BLOCK WHEN WE CLOSE THE FILE

	HP_block_info* curr_HP_block_info =  data + BF_BLOCK_SIZE - sizeof(HP_block_info);	//WRITE THE HP_BLOCK_INFO AT THE END
		curr_HP_block_info->key_attribute = ID;		//SPECIAL NODE
		curr_HP_block_info->next_block_address = NULL;
		curr_HP_block_info->records_num = 7; // > 6 TRICK



	BF_Block_SetDirty(first_block);					//SET DIRTY AS MODIFIED
	BF_UnpinBlock(first_block);						//UNPIN BLOCK AS WE DO NOT NEED IT
	
	return 0;
}

HP_info* HP_OpenFile(char *fileName){	
	int file_describer;
	if (BF_OpenFile(fileName, &file_describer) != BF_OK)return NULL;	//OPEN FILES

	BF_Block * first_block;	
	BF_Block_Init(&first_block);	//ALLOCATE A BLOCK TO EXCTRACT THE SPECIAL(1st) BLOCK FROM THE FILE

	if (BF_GetBlock(file_describer,0,first_block) != BF_OK)return NULL;	//ASSIGN 1st BLOCK

	HP_info * curr_HP_info;	//DEFINE HP_info POINTER
	void * data = BF_Block_GetData(first_block);	//EXTRACT BLOCK'S DATA A.K.A SPECIAL INFO
	curr_HP_info = (HP_info *)data;			//STORE THE SPECIAL INFO IN THE HP_Info POINTER

	if(!curr_HP_info->is_heap_file)return NULL;	//if not a heap file => error

	curr_HP_info->file_describer = file_describer;
	
	BF_Block_SetDirty(first_block);					//SET DIRTY AS MODIFIED
	return curr_HP_info;

}


int HP_CloseFile(HP_info* curr_HP_info ){
	
	BF_Block_SetDirty(curr_HP_info->storage_location);
	BF_UnpinBlock(curr_HP_info->storage_location);


	CALL_BF(BF_CloseFile(curr_HP_info->file_describer));	//CLOSE FILE
	CALL_BF(BF_Close());							//CLOSE BUFFER => SAVE IN THE DISK
	return 0;

}

int HP_InsertEntry(HP_info *curr_HP_info, Record record){
	BF_Block* curr_block;

	BF_Block_Init(&curr_block);

	int last_block_id = 0;
	

	CALL_BF(BF_GetBlockCounter(curr_HP_info->file_describer,&last_block_id))
	last_block_id--;		//GET LAST BLOCK'S ID
	
	CALL_BF(BF_GetBlock(curr_HP_info->file_describer,last_block_id,curr_block))	//GET THE LAST RECORD OF THE FILE



	void* curr_data = BF_Block_GetData(curr_block);	//GET ITS DATA ADDRESS
    
	HP_block_info* curr_HP_block_info = curr_data + BF_BLOCK_SIZE - sizeof(HP_block_info);	//GET ITS INFO
	
	if(curr_HP_block_info->records_num >= curr_HP_info->block_capacity_of_records){	//IF THE RECORD CANNOT BE STORED IN THE LAST BLOCK
		//CREATE A NEW ONE AND APPEND IT TO THE FILE
		
		BF_Block* util_block;		//CREATE A UTILITY BLOCK
		BF_Block_Init(&util_block);
	
		BF_AllocateBlock(curr_HP_info->file_describer,util_block);	//ALLOCATE IT TO THE FILE

		void* util_data = BF_Block_GetData(util_block);	//GET THE DATA ADDRESS
    	
		Record* record_slot = util_data;		//WRITE THE RECORD AT THE BEGINNING
		record_slot[0] = record;

		HP_block_info* util_HP_block_info =  util_data + BF_BLOCK_SIZE - sizeof(HP_block_info);	//WRITE THE HP_BLOCK_INFO AT THE END
		util_HP_block_info->key_attribute = record.id;	
		util_HP_block_info->next_block_address = NULL;
		util_HP_block_info->records_num = 1;
		
		//UPDATE THE CURRENT BLOCK'S INFO AND THE HP INFO
		
		curr_HP_block_info->next_block_address = util_block;	//CURRENT BLOCK'S INFO
		
		curr_HP_info->last_block_address = util_block;			//HP_INFO


		BF_Block_SetDirty(util_block);					//SET DIRTY AS MODIFIED
		BF_UnpinBlock(util_block);						//UNPIN BLOCK AS WE DO NOT NEED IT

		// BF_Block_Destroy(&util_block);

	}
	else{
		
		int index_of_the_last_rec = curr_HP_block_info->records_num;

		Record* record_slot = curr_data;		//WRITE THE RECORD AT THE BEGINNING
		record_slot[index_of_the_last_rec] = record;

		curr_HP_block_info->records_num++;	//UPDATE THE BLOCK'S INFO

	}		

	BF_Block_SetDirty(curr_block);					//SET DIRTY AS MODIFIED
	BF_UnpinBlock(curr_block);						//UNPIN BLOCK AS WE DO NOT NEED IT
	BF_Block_Destroy(&curr_block);					//DESTROY BLOCK

	return 0;


}

int HP_GetAllEntries(HP_info *curr_HP_info, int value){

 	BF_Block *block;			//ALLOCATE SPACE FOR A BLOCK
  	BF_Block_Init(&block);
	

	int blocks_num;	
	CALL_BF(BF_GetBlockCounter(curr_HP_info->file_describer,&blocks_num))	//GET THE NUM OF BLOCKS IN THE FILE(COUNTING THE 1st)


  	int fd  = curr_HP_info->file_describer;		//
												//
	void* data;									//
	HP_block_info* curr_HP_block_info;			//		SOME VARIABLES FOR CLEANER CODE
												//
	int recs_num;								//
												//
	int i;										//


  	for (i = 1; i < blocks_num - 1; i++) {	//FOR EACH BLOCK AFTER THE 1st
		
    	CALL_BF(BF_GetBlock(fd, i, block));		//GET IT
    	
		data = BF_Block_GetData(block);		//GET ITS DATA

    	Record* rec = data;													//ASSIGN THE DATA ADDRESS TO A RECORD POINTER TO HANDLE THE RECORDS
		curr_HP_block_info = data + BF_BLOCK_SIZE - sizeof(HP_block_info);	//GET THE BLOCK'S INFO
		recs_num = curr_HP_block_info->records_num;							//EXTRACT FORM THAT HOW MANY RECORDS ARE INSIDE THE BLOCK

		for(int j = 0; j < recs_num;j++){		//FOR EACH RECORD
			if(rec[j].id == value){				//IF ITS ID IS EQUAL TO THE GIVEN VALUE
			    printRecord(rec[j]);			//PRINT IT
				CALL_BF(BF_UnpinBlock(block));	//
				BF_Block_Destroy(&block);		//DEALLOCATE THE SPACE

				return i;						//RETURN THE ID OF THE BLOCK IN WHICH IT WAS FOUND
			}
		}
		CALL_BF(BF_UnpinBlock(block));			//WE WANT AFTER EACH ITERATION THE MRU THAT WE DO NOT NEED TO BE REPLACED
  	}

	BF_Block_Destroy(&block);					//DEALLOCATE THE SPACE IF VALUE WAS NOT FOUND IN KEYS


	return 0;
}


void HP_info_Print(HP_info * curr_HP_info){
printf("curr_HP_info->filename: %s\n",curr_HP_info->filename);
printf("curr_HP_info->file_describer: %d\n",curr_HP_info->file_describer);
printf("curr_HP_info->is_heap_file: %d\n",curr_HP_info->is_heap_file);	
printf("curr_HP_info->last_block_address: %p\n",curr_HP_info->last_block_address);		
printf("curr_HP_info->block_capacity_of_records: %d\n",curr_HP_info->block_capacity_of_records);
}
