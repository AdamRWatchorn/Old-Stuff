#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/errno.h>
#include "ext2.h"
#include "helper.h"

/*
This program takes three command line arguments. The first is the name of an ext2 formatted virtual disk.
 The second is the path to a file on your native operating system, and the third is an absolute path on your ext2
 formatted disk. The program should work like cp, copying the file on your native file system onto the specified
 location on the disk. If the specified file or target location does not exist, then your program should return
 the appropriate error (ENOENT). Please read the specifications of ext2 carefully, some things you will not need
 to worry about (like permissions, gid, uid, etc.), while setting other information in the inodes
 may be important (e.g., i_dtime).
*/

unsigned char *disk;

char* create_buffer_of_file(char *path_of_file);

int main(int argc, char **argv) {

    if(argc != 4) {
        fprintf(stderr, "Invalid number of arguments");
        exit(1);
    }

    char *disk_name = argv[1];
    char *path_of_file_to_copy = argv[2];
    char *path_to_place_file =  argv[3];

    if (path_to_place_file[0] != '/')
        fprintf(stderr, "You must provide an absolute path.\n");

	FILE *fp = fopen(path_of_file_to_copy, "r");
    if (fp == NULL) {
    	printf("File not found\n");
    	exit(ENOENT);
    }
	
    // Image file maps to virtual memory.
    int fd = open(disk_name, O_RDWR);
	int source_fd = open(path_of_file_to_copy, O_RDONLY);
	   
	// source file not found
	if (source_fd == -1) {
		errno = ENOENT;
		perror(path_of_file_to_copy);
		exit(errno);
	}
    // file size
	int fsize = lseek(source_fd, 0, SEEK_END);
	
	int num_blocks = (fsize  / EXT2_BLOCK_SIZE ) + 1;
	
    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }	

    // get the group descriptor table
    struct ext2_group_desc *group_des_table = (struct ext2_group_desc*)(disk + EXT2_BLOCK_SIZE * 2);

    int inode_bitmap = group_des_table->bg_inode_bitmap;
    //int block_bitmap = group_des_table->bg_block_bitmap;
    int inode_table = group_des_table->bg_inode_table;

    struct ext2_inode* table = (struct ext2_inode*)(disk + (inode_table * EXT2_BLOCK_SIZE));

    void *inodes = disk + EXT2_BLOCK_SIZE * inode_table;

 	// get parent path.
    int i;
    char *parentPath = malloc(sizeof(char) * strlen(path_to_place_file));
    strncpy(parentPath, path_to_place_file, strlen(path_to_place_file));
    for (i = strlen(path_to_place_file) - 2; i >= 0; i--)
    {
        if (parentPath[i] == '/') {
            parentPath[i + 1] = '\0';
            break;
        }
    }

    // get node index of the parent directory.
    int parent_inode_num = get_inode_index(parentPath, table);
    if (parent_inode_num == -1){
	fprintf(stderr, "No parent directory\n");
    	exit(ENOENT);
    }
	
	// get the inode of that parent directory
    struct ext2_inode *parent_inode = inodes +  sizeof(struct ext2_inode) * (parent_inode_num -1);
	
	//unsigned long block_bitmap_block = block_bitmap * EXT2_BLOCK_SIZE;
    //void * block_bits = (void *)(disk + block_bitmap_block);
	 
    // get free inode number
    unsigned long inode_bitmap_block = inode_bitmap * EXT2_BLOCK_SIZE;
    void * inode_bits = (void *)(disk + inode_bitmap_block);

    int free_inode_index = get_free_inode(inode_bits);
	if (free_inode_index == -1) {
		perror("no free inode");
		exit(ENOSPC);
  }
	
    // populate new inode
	struct ext2_inode *new_inode = inodes + sizeof(struct ext2_inode) * free_inode_index;
    new_inode->i_mode = EXT2_S_IFDIR;
    new_inode->i_size = fsize;
    new_inode->i_blocks = num_blocks * 2;
    new_inode->i_links_count = 1;
	
	// copy file content from buffer
    for (i = 0; i < num_blocks; i++){
		// copy 
		// update each new_inode->i_block[i]
	}

	// add directory entry to the parent directory.
    int current_size = 0;
    while (current_size < EXT2_BLOCK_SIZE){
    	struct ext2_dir_entry_2 *node_dir = (struct ext2_dir_entry_2 *)
                (disk + (EXT2_BLOCK_SIZE * parent_inode->i_block[0]) + current_size);

    	if (current_size + node_dir->rec_len == EXT2_BLOCK_SIZE){
    		// this is the last block in the directory
			int new_rec_len = node_dir->rec_len - node_dir->name_len - 8;
			node_dir->rec_len = node_dir->name_len + 8;

		    // create directory entry
		    struct ext2_dir_entry_2 *par_dir = (struct ext2_dir_entry_2 *)
                    (disk + (EXT2_BLOCK_SIZE * parent_inode->i_block[0] 
                        + current_size + node_dir->rec_len));

		    par_dir->inode = free_inode_index + 1;
		    par_dir->file_type = 2;

		    // get the new name
		    char dup_path[strlen(path_of_file_to_copy)];
		    strcpy(dup_path, path_of_file_to_copy);
		    char *ret;
		    ret = strrchr(dup_path, '/');
		    if (ret[0] == '/'){
		    	ret++;
		    }

		    par_dir->rec_len = new_rec_len;
		    par_dir->name_len = strlen(ret);
		    strcpy(par_dir->name,ret);

		    break;
    	}
    	current_size += node_dir->rec_len;
    }
		return 0;	
}

char* create_buffer_of_file(char *path_of_file){
    FILE *file_to_copy = fopen(path_of_file, "r");

    if(file_to_copy == NULL || fseek(file_to_copy, 0, SEEK_END) != 0){
        //TODO proper error
        fprintf(stderr, "could not find or open file");
        exit(1);
    }

    long buffer_size = 0;
    if((buffer_size = ftell(file_to_copy)) == -1){
        //TODO proper error
        fprintf(stderr, "get file size");
        exit(1);
    }

    char *buffer = malloc(sizeof(char) * buffer_size + 1);

    if(fseek(file_to_copy, 0, SEEK_SET) != 0){
        //TODO proper error
        fprintf(stderr, "get file size");
        exit(1);
    }

    size_t length = fread(buffer, sizeof(char), (size_t) buffer_size, file_to_copy);
    buffer[length] = '\0';
    if(ferror(file_to_copy) != 0){
        //TODO proper error
        fprintf(stderr, "get file size");
        exit(1);
    }

    fclose(file_to_copy);
    return buffer;
}