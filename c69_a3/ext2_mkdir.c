#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/errno.h>
#include <stdbool.h>
#include "ext2.h"
#include "helper.h"

unsigned char *disk;

int main(int argc, char **argv){

    if(argc != 3) {
	fprintf(stderr, "Invalid number of arguments");
	exit(1);
    }

    if (argv[2][0] != '/') {
	fprintf(stderr, "You must provide an absolute path.\n");
    }

    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * EXT2_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // get the group descriptor table (gd)
    struct ext2_group_desc *desc_block = (struct ext2_group_desc *)(disk + (EXT2_BLOCK_SIZE * 2));

    int inode_bitmap = desc_block->bg_inode_bitmap;
    int block_bitmap = desc_block->bg_block_bitmap;
    int inode_table = desc_block->bg_inode_table;

    struct ext2_inode* table = (struct ext2_inode *)(disk + (inode_table * EXT2_BLOCK_SIZE));

    //Constructs proper string representing the path
    int pathArg = 2;
    int len = strlen(argv[pathArg]); //length of path
    char argPath[len+1];
    memcpy(argPath, argv[pathArg],len ); // Compiles variables
    argPath[len] = '\0'; // Guarantees string termination

    // if index is not -1, then it already exists.
    if (get_inode_index(argPath, table) != -1){
        fprintf(stderr, "Directory already exists\n");
        exit(EEXIST);
    }

    // Get parent path.
    int i;
    char *parentPath = malloc(sizeof(char) * len);
    strncpy(parentPath, argPath, len);
    for (i = len - 2; i >= 0; i--){
        if (parentPath[i] == '/') {
            if (i == 0){
                parentPath[i + 1] = '\0';
                break;
            }else {
                parentPath[i] = '\0';
		            break;
	          }
        }
    }

    // get node index of the parent directory
    int parent_inode_num = get_inode_index(parentPath, table);

    if (parent_inode_num == -1){
        fprintf(stderr, "No parent directory\n");
    	exit(ENOENT);
    }

    // get the inode of that parent directory
    struct ext2_inode *parent_inode = (struct ext2_inode *)(table + (parent_inode_num-1));

    // make new directory
    unsigned long block_bitmap_block = block_bitmap * EXT2_BLOCK_SIZE;
    void * block_bits = (void *)(disk + block_bitmap_block);

    int free_block = get_free_block(block_bits);

    if (free_block == -1) {
            perror("no free block");
  	    exit(ENOSPC);
    }

    // get free inode number
    unsigned long inode_bitmap_block = inode_bitmap * EXT2_BLOCK_SIZE;
    void * inode_bits = (void *)(disk + inode_bitmap_block);

    int free_inode_index = get_free_inode(inode_bits);
    if (free_inode_index == -1) {
        perror("no free inode");
        exit(ENOSPC);
    }

    // create new inode
    struct ext2_inode *new_inode = (struct ext2_inode *)(table + free_inode_index);

    new_inode->i_mode = EXT2_S_IFDIR;
    new_inode->i_size = EXT2_BLOCK_SIZE;
    new_inode->i_blocks = 2;
    new_inode->i_links_count = 2;
    new_inode->i_block[0] = free_block;

    // create new directory entry for itself
    struct ext2_dir_entry_2 *new_dir_curr = (struct ext2_dir_entry_2 *)(disk + (EXT2_BLOCK_SIZE * free_block));
    new_dir_curr->inode = free_inode_index;
    new_dir_curr->rec_len = 12;
    new_dir_curr->name_len = 1;
    new_dir_curr->file_type = EXT2_FT_DIR;
    strcpy(new_dir_curr->name, ".");

    // create new directory entry for parent
    struct ext2_dir_entry_2 *new_dir_parent = (struct ext2_dir_entry_2 *)(disk + (EXT2_BLOCK_SIZE * free_block)+ 12);

    new_dir_parent->inode = parent_inode_num;
    new_dir_parent->rec_len = EXT2_BLOCK_SIZE - 12;
    new_dir_parent->name_len = 2;
    new_dir_parent->file_type = EXT2_FT_DIR;
    strcpy(new_dir_parent->name, "..");

    // get the last directory block of its parent's directory
    int k = 0;
    while (parent_inode->i_block[k+1]){
        k++;
    }

    // add directory entry to the parent directory
    int current_size = 0;
    while (current_size < EXT2_BLOCK_SIZE){
    	struct ext2_dir_entry_2 *node_dir = (struct ext2_dir_entry_2 *)
                (disk + (EXT2_BLOCK_SIZE * parent_inode->i_block[k]) + current_size);

    	if (current_size + node_dir->rec_len == EXT2_BLOCK_SIZE){
    	    // this is the last block in the directory

            int adjusted = (node_dir->name_len + 8) + 4 - (node_dir->name_len + 8 )% 4;
            int new_rec_len = node_dir->rec_len - adjusted;

            node_dir->rec_len = adjusted;

	    // create directory entry
	    struct ext2_dir_entry_2 *par_dir = (struct ext2_dir_entry_2 *)
                    (disk + (EXT2_BLOCK_SIZE * parent_inode->i_block[k]
                        + current_size + node_dir->rec_len));

	    par_dir->inode = free_inode_index + 1;
	    par_dir->file_type = EXT2_FT_DIR;

	    // get the new name
	    char dup_path[len];
	    strcpy(dup_path, argPath);
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
