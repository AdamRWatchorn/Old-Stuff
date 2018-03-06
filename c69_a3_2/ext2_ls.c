#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#define I_MODE_MASK 0xF000
#include "ext2.h"
#include "helper.h"
unsigned char *disk;

int main(int argc, char **argv){
   int pathArg = 2;
   bool aFlag=false;
   if (argc < 3 || argc > 4) {
    fprintf(stderr, "Invalid number of Arguments\n");
    exit(1);
  }
  if (argc == 4 && (strcmp(argv[3],"-a") !=0  && strcmp(argv[2],"-a") !=0)){
        fprintf(stderr, "Invalid num of args \n");
    exit(1);
  }
  if (argc == 4 ){
   aFlag=true;
   if(strcmp(argv[2],"-a") == 0){
    pathArg=3;
   }
  }
  int len = strlen(argv[pathArg]);
  char argPath[len+1];
  memcpy(argPath, argv[pathArg],len );
  argPath[len] = '\0';

  //strcpy(argv[2], argPath);
  
  
  
  if (argPath[0] !='/'){
      fprintf(stderr, "Not an absolute path\n");
      exit(1);
  }
  if (strlen(argPath)> 1 && argPath[strlen(argPath) -1] == '/') {
    argPath[strlen(argPath) -1] ='\0';
  }
  
   int fd = open(argv[1], O_RDWR);

  disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (disk == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }


  struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + (EXT2_BLOCK_SIZE * 2));
  struct ext2_inode* inode_table = (struct ext2_inode*)(disk + (gd->bg_inode_table * EXT2_BLOCK_SIZE));
  //void *begin = disk + EXT2_BLOCK_SIZE*inode_table;
  /// need to find out Inode of file/dir/link
  int inode = get_inode_index(argPath,inode_table);
  if(inode == -1)
  {
   perror("No Such File or directory");
   return ENOENT;
  }
  inode -=1;
	//detect type
  if((inode_table[inode].i_mode & I_MODE_MASK)==EXT2_S_IFREG || (inode_table[inode].i_mode & I_MODE_MASK)==EXT2_S_IFLNK){

   char *last = strrchr(argPath, '/');
   printf("%s\n",last+1);
  }else if((inode_table[inode].i_mode & I_MODE_MASK) == EXT2_S_IFDIR){
   int blockIterator;
   int totalSize =0;
   for (blockIterator=0;blockIterator<13;blockIterator++){
    int blockNum = inode_table[inode].i_block[blockIterator];
    int inode_size = inode_table[inode].i_size;
    
    if(blockIterator==12)
    {
	//list files from indirect blocks
		if (inode_table[inode].i_block[12]) {
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
					if(!aFlag && (strcmp(name,".")==0 ||strcmp(name,"..")==0)){
						printf("\n");      
					} else{
						printf("%s\n",name);   
					}
               		
                	idx += dir->rec_len;	
                	dir_ptr += dir->rec_len;
           	    }

            }
                

           }
	}

     
    }else
    {
	// list files from direct pointers
     struct ext2_dir_entry_2* item = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * blockNum);
     while(totalSize<inode_size){

       char name[255+1];
       memcpy(name, item ->name, item ->name_len);
       name[item ->name_len] = '\0';
       if(!aFlag && (strcmp(name,".")==0 ||strcmp(name,"..")==0)){
        printf("\n");      
       } else{
        printf("%s\n",name);   
       }
              
       totalSize += item->rec_len;
       item = (void *)item + item->rec_len;
       if((totalSize % EXT2_BLOCK_SIZE == 0)){
		break;
       }
      
     }
    }

    
   }
   
  }
  return 0;
}
