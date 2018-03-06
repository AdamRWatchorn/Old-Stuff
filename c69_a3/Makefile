all: helper.c ext2_ls.c ext2_cp.c ext2_mkdir.c ext2_ln.c ext2_rm.c
	gcc -Wall -o ext2_ls helper.c ext2_ls.c
	gcc -Wall -o ext2_cp helper.c ext2_cp.c
	gcc -Wall -o ext2_mkdir helper.c ext2_mkdir.c
	gcc -Wall -o ext2_ln helper.c ext2_ln.c
	gcc -Wall -o ext2_rm helper.c ext2_rm.c

ext2_ls: helper.c ext2_ls.c
	gcc -Wall -o ext2_ls helper.c ext2_ls.c

ext2_cp: helper.c ext2_cp.c
	gcc -Wall -o ext2_cp helper.c ext2_cp.c

ext2_mkdir: helper.c ext2_mkdir.c
	gcc -Wall -o ext2_mkdir helper.c ext2_mkdir.c

ext2_ln: helper.c ext2_ln.c
	gcc -Wall -o ext2_ln helper.c ext2_ln.c

ext2_rm: helper.c ext2_rm.c
	gcc -Wall -o ext2_rm helper.c ext2_rm.c

clean :
		rm ext2_ls ext2_rm ext2_cp ext2_mkdir ext2_ln
