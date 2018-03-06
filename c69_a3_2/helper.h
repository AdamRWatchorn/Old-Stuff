

int get_inode_index(char fullPath[],struct ext2_inode* inode_table);
int get_next_index(char path[]);
int get_free_inode(void * inode_map);
int get_free_block(void * blk_map);
int free_blocks(void* start);
int get_inode(void* start);
int get_block(void* start);
void set_bitmap(void* start, int i, int set) ;

