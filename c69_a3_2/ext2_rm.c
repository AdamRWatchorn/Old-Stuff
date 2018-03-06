#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include "ext2.h"
#include "helper.h"

/*
This program takes two command line arguments. The first is the name of an ext2 formatted virtual disk,
 and the second is an absolute path to a file or link (not a directory) on that disk. The program should
 work like rm, removing the specified file from the disk. If the file does not exist or if it is a
 directory, then your program should return the appropriate error. Once again, please read the
 specifications of ext2 carefully, to figure out what needs to actually happen when a file or link is
 removed (e.g., no need to zero out data blocks, must set i_dtime in the inode, removing a directory entry
 need not shift the directory entries after the one being deleted, etc.).
*/

unsigned char *disk;
#define I_MODE_MASK 0xF000


int main(int argc, char **argv) {

    if(argc != 3) {
        fprintf(stderr, "Invalid number of arguments");
        exit(1);
    }

    char *disk_name = argv[1];
    char *path_of_file_to_delete = argv[2];
    char *path_of_parent_directory = strdup(path_of_file_to_delete);
    *(strrchr(path_of_parent_directory, '/')) = '/';
    *(strrchr(path_of_parent_directory, '/') + 1) = '\0';

    if (path_of_file_to_delete[0] != '/') {
        fprintf(stderr, "You must provide an absolute path.\n");
        exit(1);
    }

    // Image file maps to virtual memory.
    int fd = open(disk_name, O_RDWR);
    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + (EXT2_BLOCK_SIZE * 2));
    struct ext2_inode* inode_table = (struct ext2_inode*)(disk + (gd->bg_inode_table * EXT2_BLOCK_SIZE));

    //this refers to gman's function in ext2_ls.c... need to link or copy function
    int parent_inode_i = get_inode_index(path_of_parent_directory, inode_table) - 1;
    int inode_i = get_inode_index(path_of_file_to_delete, inode_table) - 1;

    if(inode_i < -1 || parent_inode_i < -1) {
        printf("No Such File or directory");
        exit(ENOENT);
    }

    //ensure it is file or link
    if((inode_table[inode_i].i_mode & I_MODE_MASK) != EXT2_S_IFREG && (inode_table[inode_i].i_mode & I_MODE_MASK) != EXT2_S_IFLNK) {
        printf("path is not file or link");
        exit(1);
    }

    //set delete time to now
    inode_table[inode_i].i_dtime = (unsigned int) time(0);

    //decrease link count
    inode_table[inode_i].i_links_count -= 1;

    //find file in parent's dir_entry and set that entry->inode = 0;
    //also, if file is not very begining then extend the length of previous file
    //to point to next file
    int total_size = 0;
    int i = 0;
    for(i=0; i<12; i++){
        int block_number = inode_table[parent_inode_i].i_block[i];
        struct ext2_dir_entry_2* directory_item = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * block_number);
        if(directory_item->inode == inode_i + 1)
            directory_item->inode = 0;
        else{
            while(total_size < inode_table[parent_inode_i].i_size){
                struct ext2_dir_entry_2* next_directory_item = (void *) directory_item + directory_item->rec_len;
                total_size += next_directory_item->rec_len;

                if(next_directory_item->inode == inode_i + 1){
                    next_directory_item->inode = 0;
                    directory_item->rec_len += next_directory_item->rec_len;
                    i = 12; break;
                }
                directory_item = next_directory_item;
                if (total_size % EXT2_BLOCK_SIZE  == 0)
                    break;
            }
        }
    }

    //split path into dir and file
    //get inode of parent
    //loop though the inode vblocks
    //dont forget to subtract 1
    // get inode of file
    //if it odesnt exist or is a dir return error
    // follow the dir entry in the parent path
    //3 cases
    //1. only dir entry in block. ==BLOCK SIZIE ==reclen
    //2.first dir entry in block ask daniel 2 not too sure
    //3. norma case let the previous dir entry get more space ie add to rec len

    //if file doesnt exist ENONENT
    //IF file found
    // decrement inode link count for file
    //clear all blocks and set them to 0.
    //normal + single indirection
    //set dtime
    //update bitmaps block + inode

    // the end update group dec table free_blocks + free inode count.
    return 0;
}
