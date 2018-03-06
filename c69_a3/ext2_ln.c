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

		//reverse through string to find parent path
		char *parent(char path[]) {
			int i;
			char dirPath[strlen(path)+1];
			strcpy(dirPath, path);
			//do we get the null terminaited char?
			for (i = strlen(path) - 1; i > 0; i--)
			{
				if (dirPath[i] == '/') {
					dirPath[i] = '\0';
					break;
				}
			}
			char* fpath = malloc(sizeof (char) * (strlen(dirPath)+1));
			strcpy(fpath, dirPath);
			fpath[(strlen(dirPath)+1)] ='\0';
			if(strlen(path)==strlen(dirPath)){
				fpath[1] ='\0';
			}


			return fpath;
		}

		int main(int argc, char **argv){
		   int srcArg = 2;
		   int destArg = 3;
		   bool sFlag=false;
		   if (argc < 4 || argc > 5) {
			fprintf(stderr, "Invalid number of Arguments\n");
			exit(1);
		  }
		  // Determine if -s is enabled and where
		  if (argc == 5 && (strcmp(argv[3],"-s") !=0  && strcmp(argv[2],"-s") !=0&& strcmp(argv[4],"-s") !=0)){
				fprintf(stderr, "Invalid num of args \n");
			exit(1);
		  }
		  if (argc == 5 ){
		   sFlag=true;
		   if(strcmp(argv[2],"-s") == 0){
			srcArg=3;
			destArg=4;
		   }
		    if(strcmp(argv[3],"-s") == 0){
			srcArg=2;
			destArg=4;
		   }
		    if(strcmp(argv[4],"-s") == 0){
			srcArg=2;
			destArg=3;
		   }
		  }
		  // store src path and dest path
		  int srclen = strlen(argv[srcArg]);
		  char srcPath[srclen+1];
		  memcpy(srcPath, argv[srcArg],srclen );
		  srcPath[srclen] = '\0';

		  int destlen = strlen(argv[destArg]);
		  char destPath[destlen+1];
		  memcpy(destPath, argv[destArg],destlen );
		  destPath[destlen] = '\0';



		  if (srcPath[0] !='/' || destPath[0] !='/'){
			  fprintf(stderr, "Not an absolute path\n");
			  exit(1);
		  }
		  //strip traling '/'
		  if (strlen(srcPath)> 1 && srcPath[strlen(srcPath) -1] == '/') {
			srcPath[strlen(srcPath) -1] ='\0';
		  }
		  if (strlen(destPath)> 1 && destPath[strlen(destPath) -1] == '/') {
			destPath[strlen(destPath) -1] ='\0';
		  }

		   int fd = open(argv[1], O_RDWR);

		  disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		  if (disk == MAP_FAILED) {
			perror("mmap");
			exit(1);
		  }

		  //split src and dest path into parentPaths and files
		  char* destDir = parent(destPath);
		  char* srcDir = parent(srcPath);
		  char *lastD = strrchr(destPath, '/');
		  char *lastS = strrchr(srcPath, '/');
		  char* destFile =lastD+1;
		  char* srcFile = lastS+1;

		 if(strlen(srcFile) == 0){
			   perror("source path is a dir.");

			   exit(EISDIR);
		  }
		  if(strlen(destFile) == 0){
			   perror("Destination path is a dir.");

			   exit(EISDIR);
		  }



		  struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + (EXT2_BLOCK_SIZE * 2));
		  struct ext2_inode* inode_table = (struct ext2_inode*)(disk + (gd->bg_inode_table * EXT2_BLOCK_SIZE));
		  int srcNode = get_inode_index(srcDir,inode_table);
		  int destNode = get_inode_index(destDir,inode_table);
		  // if either parent paths arent found return an error
		  if (srcNode ==-1 || destNode ==-1) {
			fprintf(stderr, "Invalid Directory for SRC or DEST \n");
			exit(ENOENT);
		  }
		  // subtract by 1 for easy inode/ access
		  srcNode -=1;
		  destNode -=1;
		  int srcFnode=-1;


		  // Using the parent and file path
		  //iterate over the parent dir and try to find the file
		  // Source file
		  int blockIterator;
		   int totalSize =0;
		   for (blockIterator=0;blockIterator<13;blockIterator++){
			int blockNum = inode_table[srcNode].i_block[blockIterator];
			int inode_size = inode_table[srcNode].i_size;
			//indirection handling
			if(blockIterator==12)
			{
			 if (inode_table[srcNode].i_block[12]) {
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
			        if (strcmp(name,srcFile) == 0) {
				       srcFnode = dir->inode -1;
					   break;
			        }


                	idx += dir->rec_len;
                	dir_ptr += dir->rec_len;
           	    }
				if(srcFnode>-1){
						break;
					}
               }


              }
	         }

			}else
			{
			// normal searching
			 struct ext2_dir_entry_2* item = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * blockNum);
			 while(totalSize<inode_size){
			   char name[255+1];
			   memcpy(name, item ->name, item ->name_len);
			   name[item ->name_len] = '\0';
			   if (strcmp(name,srcFile) == 0) {
				srcFnode = item->inode -1;
			   }

			   totalSize += item->rec_len;
			   item = (void *)item + item->rec_len;
				   if((totalSize % EXT2_BLOCK_SIZE == 0)){
				break;
				   }
			 }
			}
		   }
		   if (srcFnode == -1) {
			  fprintf(stderr, "Invalid Source path \n");
			  return(ENOENT);
		  }

		  if((inode_table[srcFnode].i_mode & I_MODE_MASK) == EXT2_S_IFDIR){
		   	fprintf(stderr, "Source Path refers to a directory \n");
		   	return(EISDIR);
		  }

		  // Check dest file dir
		  int destFnode=-1;
		  // TRY and file src file

		   totalSize =0;
		   for (blockIterator=0;blockIterator<13;blockIterator++){
			int blockNum = inode_table[destNode].i_block[blockIterator];
			int inode_size = inode_table[destNode].i_size;

			if(blockIterator==12)
			{
			 if (inode_table[destNode].i_block[12]) {
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
			        if (strcmp(name,destFile) == 0) {
				       destFnode = dir->inode -1;
					   break;
			        }

					if(destFnode>-1){
						break;
					}
                	idx += dir->rec_len;
                	dir_ptr += dir->rec_len;
           	    }

               }


              }
	         }

			}else
			{
			 struct ext2_dir_entry_2* item = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * blockNum);
			 while(totalSize<inode_size){
			   char name[255+1];
			   memcpy(name, item ->name, item ->name_len);
			   name[item ->name_len] = '\0';
			   if (strcmp(name,destFile) == 0) {
				destFnode = item->inode -1;
				break;
			   }

			   totalSize += item->rec_len;
			   item = (void *)item + item->rec_len;
			   if((totalSize % EXT2_BLOCK_SIZE == 0)){
				break;
				   }
			 }
			}
		   }
		   if (destFnode != -1) {
			  fprintf(stderr, "Invalid Destination Path\n");
			  if((inode_table[srcFnode].i_mode & I_MODE_MASK) == EXT2_S_IFDIR){
			   exit(EISDIR);
			  } else{
			   exit(ENOENT);
			  }
		  }

		  //Create sym link -- if path < 60 no block required
		  // else create block
		  if (sFlag){
			// get free inode number
			unsigned long inode_bitmap_block = gd->bg_inode_bitmap * EXT2_BLOCK_SIZE;
			void * inode_bits = (void *)(disk + inode_bitmap_block);

			int free_inode;
			free_inode = get_free_inode(inode_bits);
			if (free_inode == -1) {
			  perror("no free block for new inode");
			  exit(ENOSPC);
			}

			unsigned long block_bitmap_block = gd->bg_block_bitmap * EXT2_BLOCK_SIZE;
			void * block_bits = (void *)(disk + block_bitmap_block);
			int free_block = get_free_block(block_bits);
			if (free_block == -1) {
			  perror("no free block for new directory entry");
			  exit(ENOSPC);
			}

			// create new inode
			inode_table[free_inode-1].i_mode = EXT2_S_IFLNK;
			inode_table[free_inode-1].i_size = EXT2_BLOCK_SIZE;
			inode_table[free_inode-1].i_blocks = 0;
			inode_table[free_inode-1].i_links_count = 0;
			//fast symlink
			if(strlen(srcPath)<60){
				memset(inode_table[free_inode-1].i_block, 0, sizeof inode_table[free_inode-1].i_block);
				memcpy(inode_table[free_inode-1].i_block, srcPath, strlen(srcPath));

			}else{
			//need to allocate block - store the path in the blocks
			//regular symlink
			int tmplen = strlen(srcPath);
			int numblocks = 0;
			//calc number of blocks
			if (tmplen % EXT2_BLOCK_SIZE ==0){
				numblocks = tmplen/EXT2_BLOCK_SIZE;
			}else{
				numblocks = tmplen/EXT2_BLOCK_SIZE +1;

			}
			//check for indirect
			if (numblocks > 12) {
				numblocks++;
			}
			//not enough blocks
			if (free_blocks(block_bits)< numblocks){
			  perror("no free block for new directory entry");
			  exit(ENOSPC);
			}
			//transfer path into blocks
			int toTransfer = tmplen;
			int transferred = 0;
			int i;
			int blocks[numblocks];
			char *pathPtr;
			pathPtr = srcPath;
			for (i = 0; i < numblocks && i<12; i++) {
			  blocks[i] = get_free_block(block_bits);
			  if (toTransfer < EXT2_BLOCK_SIZE) {
				memcpy(disk + EXT2_BLOCK_SIZE * (blocks[i]),
				pathPtr + transferred, toTransfer);
				toTransfer =0;
			  } else {
				memcpy(disk + EXT2_BLOCK_SIZE * (blocks[i]),
				pathPtr + transferred, EXT2_BLOCK_SIZE);
				toTransfer -= EXT2_BLOCK_SIZE;
				transferred += EXT2_BLOCK_SIZE;
			  }
			  if (toTransfer==0){
				break;
			  }


			}
			if (toTransfer!=0){
			  blocks[12] = get_free_block(block_bits);
			  int* master = (void*)disk + EXT2_BLOCK_SIZE * (blocks[12]);
			  for (i = 13; i < numblocks; i++) {
				blocks[i] = get_free_block(block_bits);

				if (toTransfer < EXT2_BLOCK_SIZE) {
					memcpy(disk + EXT2_BLOCK_SIZE * (blocks[i]),
					pathPtr + transferred, toTransfer);
					toTransfer =0;
				} else {
					memcpy(disk + EXT2_BLOCK_SIZE * (blocks[i]),
					pathPtr + transferred, EXT2_BLOCK_SIZE);
					toTransfer -= EXT2_BLOCK_SIZE;
					transferred += EXT2_BLOCK_SIZE;
				}
				if (toTransfer==0){
					break;
				}
				//store addresses
				*master = blocks[i];
				//next element
				master++;
			  }

			}
			//add blocks to iblocks
			for (i = 0; i < numblocks && i<13; i++) {
				inode_table[free_inode-1].i_block[i] = blocks[i];
			}
			inode_table[free_inode-1].i_blocks = i;
		   }
		   int fblock = get_free_block(block_bits);
		   if (fblock == -1) {
			  perror("no free block for new block");
			  exit(ENOSPC);
		   }
		   //new dir entry in parent
		   struct ext2_dir_entry_2* item = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE *fblock);
		   item->inode = free_inode;
		   item->rec_len = EXT2_BLOCK_SIZE;
		   item->name_len = strlen(destFile);
		   item->file_type = EXT2_FT_SYMLINK;
		   strncpy((char*)item + 8, destFile, strlen(destFile));
		   inode_table[free_inode-1].i_links_count++;
		   int pBlocks =0;
		   if (inode_table[destNode].i_size % EXT2_BLOCK_SIZE == 0){
				pBlocks = inode_table[destNode].i_size/EXT2_BLOCK_SIZE;
			}else{
				pBlocks = inode_table[destNode].i_size/EXT2_BLOCK_SIZE +1;

		   }
		   // add blocks
		   if (pBlocks<12){
			  int i;
			  for (i = 0; i < 12; i++) {

					if (!inode_table[destNode].i_block[i]) {
					  inode_table[destNode].i_block[i]=fblock;
					  inode_table[destNode].i_size += EXT2_BLOCK_SIZE;
					  inode_table[destNode].i_blocks += 1;
					  break;
					}
				  }


			}else {
			//attmept to add when single indirection
					if (pBlocks==12 &&inode_table[destNode].i_block[12]==0){
						int ffblock = get_free_block(block_bits);
						if (fblock == -1) {
						  perror("no free block for new block");
						  exit(ENOSPC);
						}
						inode_table[destNode].i_block[12]=ffblock;
						inode_table[destNode].i_blocks += 1;

					}


					int *ind_ptr = (int *)(disk + (EXT2_BLOCK_SIZE * inode_table[destNode].i_block[12]));
					int j;
					for (j = 0; j < (EXT2_BLOCK_SIZE / sizeof(int)); j++) {
						if (!ind_ptr[j]) {
							 ind_ptr[j] = fblock;
							 break;
						}
					}

		   }

		  // Create hard link -- type file
		  }else{
		  //get free block
			unsigned long block_bitmap_block = gd->bg_block_bitmap * EXT2_BLOCK_SIZE;
			void * block_bits = (void *)(disk + block_bitmap_block);
			int *fblock = malloc(sizeof(int));
			fblock[0] = get_free_block(block_bits);
			if (fblock[0] == -1) {
			  perror("no free block for new block");
			  exit(ENOSPC);
			}
			//new dir entry
			struct ext2_dir_entry_2* item = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE *fblock[0]);
			item->inode = srcFnode+1;
			item->rec_len = EXT2_BLOCK_SIZE;
			item->name_len = strlen(destFile);
			item->file_type = EXT2_FT_REG_FILE;
			strncpy((void*)item + 8, destFile, strlen(destFile));
			inode_table[srcFnode].i_links_count++;
			int pBlocks =0;
			//number of blocks held by parent
			if (inode_table[destNode].i_size % EXT2_BLOCK_SIZE == 0){

				pBlocks = inode_table[destNode].i_size/EXT2_BLOCK_SIZE;
			}else{
				//overflow partial block

				pBlocks = inode_table[destNode].i_size/EXT2_BLOCK_SIZE +1;

			}
			if (pBlocks<12){
			  int i;
			  // add block
			  for (i = 0; i < 12; i++) {

					if (!inode_table[destNode].i_block[i]) {
					  inode_table[destNode].i_block[i]=fblock[0];
					  inode_table[destNode].i_size += EXT2_BLOCK_SIZE;
					  inode_table[destNode].i_blocks += 1;
					  break;
					}
				  }


			}else {
					//add ndirection block
					if (pBlocks==12 &&inode_table[destNode].i_block[12]==0){
						int *ffblock = malloc(sizeof(int));
						ffblock[0] = get_free_block(block_bits);
						if (ffblock[0] == -1) {
						  perror("no free block for new block");
						  exit(ENOSPC);
						}
						inode_table[destNode].i_block[12]=ffblock[0];
						inode_table[destNode].i_blocks += 1;


					}

					//add to indirection block
					int *ind_ptr = (int *)(disk + (EXT2_BLOCK_SIZE * inode_table[destNode].i_block[12]));
					int j;
					for (j = 0; j < (EXT2_BLOCK_SIZE / sizeof(int)); j++) {
						if (!ind_ptr[j]) {
							 ind_ptr[j] = fblock[0];
							 break;
						}
					}
		   }
		   }
		   return 0;
		}
