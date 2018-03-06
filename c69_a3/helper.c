#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/errno.h>
#include <stdbool.h>
#include "ext2.h"
#include "helper.h"

extern unsigned char* disk;

//FCN to get the next "/"
int get_next_index(char path[]){
   //root dir
   if (strlen(path)==1){
     return 0;
   }
   int i;
    for (i = 1;i < strlen(path); i++){
        if (path[i] ==  '/'){
            break;
        }
    }
    return i;
 }

// get the index of the inode of a file given the path and
int get_inode_index(char fullPath[],struct ext2_inode* inode_table){

  int inodePosition = get_next_index(fullPath);
  int inodeTracker = EXT2_ROOT_INO;
  char reducedPath[strlen(fullPath)];
  strncpy(reducedPath, fullPath,strlen(fullPath) + 1 );
  if (strcmp(fullPath, "/") == 0){
    return inodeTracker ;
  }

  while(inodePosition > 1){
   char path[inodePosition+1];
   int i;
   for (i=0;i< inodePosition;i++){
    path[i] = reducedPath[i+1];
   }

   //terminate string
   path[inodePosition -1] = '\0';
   //need to cut out selected portion from the path and handle it

   strncpy(reducedPath, reducedPath + strlen(path) + 1, strlen(reducedPath) +1  );
   bool found= false;
   int blockIterator;
   int totalSize =0;
   for (blockIterator=0;blockIterator<13&&!found;blockIterator++){
    int blockNum = inode_table[inodeTracker-1].i_block[blockIterator];
    int inode_size = inode_table[inodeTracker-1].i_size;
	 //indirection handling
		if(blockIterator==12){
		 if (inode_table[inodeTracker-1].i_block[12]) {
      int *ind_ptr = (int *)(disk + (EXT2_BLOCK_SIZE * blockNum));
      int j;
      for (j = 0; j < (EXT2_BLOCK_SIZE / sizeof(int)); j++) {
      int idx =0;
        if (ind_ptr[j]) {
          unsigned char * dir_ptr = (unsigned char *)(disk + (EXT2_BLOCK_SIZE * ind_ptr[j]));
          while (idx < EXT2_BLOCK_SIZE) {
            struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)dir_ptr;
            char name[255+1];
            memcpy(name, dir ->name, dir ->name_len);
            name[dir ->name_len] = 0;
            if (strcmp(name,path) == 0) {
              inodeTracker = dir->inode;
              found = true;
              break;
            }
          	idx += dir->rec_len;
          	dir_ptr += dir->rec_len;
          }
          if(found){
            break;
          }
        }
      }
     }
    }
    else{ // block index < 12
      struct ext2_dir_entry_2* item = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * blockNum);
      while(totalSize<inode_size && !found){

        if (strlen(path) == item->name_len && strncmp(path, item->name, item->name_len) == 0) {
          inodeTracker = item->inode;
          found = true;
        }
        totalSize += item->rec_len;
        item = (void *)item + item->rec_len;
        if((totalSize % EXT2_BLOCK_SIZE == 0)){
          break;
        }
      }
    }
    if (found){
      break;
    }
  }
  if (!found){
    return -1;
  }
  inodePosition = get_next_index(reducedPath);
  }
  return inodeTracker;
}

// return number of free blocks
int free_blocks(void* start) {
  int i;
  char* location;
  int count = 0;
  for (i = 0; i < 128; i++) {
    int counter = i / 8;
    int offset = i % 8;
    location= start + counter;
    count += (*location>> offset) & 1;
  }
  return 128 - count;
}
//return a free block and mark it as active
int get_free_block(void* start) {
  int block = get_block(start);

      if (block == -1) {
        return -1;
      }

    set_bitmap(start, block, 1);
    return block+1;

}

//return a free inode nd mark it as set
int get_free_inode(void* start) {
  int inode = get_inode(start);
      if (inode == -1) {
        return -1;
      }
    set_bitmap(start, inode, 1);
    return inode+1;
}

//get an empty inode
int get_inode(void* start) {
  int inode = -1;
  int i, counter, offset;
  for (i = 0; i < 128; i++) {
    counter = i / 8;
    offset = i % 8;
    char* begin = start + counter;
    if(((*begin >> offset) & 1)!=1){
	return i;
    }
   }
  return inode;
}
//get an empty block
int get_block(void* start) {
  int block = -1;
  int i, counter, offset;
  for (i = 0; i < 128; i++) {
    counter = i / 8;
    offset	 = i % 8;
    char* begin = start + counter;
    if(((*begin >> offset) & 1)!=1){
	return i;
    }
   }
  return block;
}
//set bitmap inode/bolck as active or inactive
void set_bitmap(void* start, int i, int set) {
  int counter = i / 8;
  int offset  = i % 8;
  char* begin = start + counter;
  *begin = (*begin & ~(1 << offset  )) | (set << offset);
}
