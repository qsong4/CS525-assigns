
525Assignment2_1 : dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o test_assign2_1.o
	gcc dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o test_assign2_1.o -o 525Assignment2_1

dberror.o : dberror.c dberror.h
	gcc -c dberror.c -o dberror.o

storage_mgr.o : storage_mgr.c storage_mgr.h
	gcc -c storage_mgr.c -o storage_mgr.o

buffer_mgr.o : buffer_mgr.c buffer_mgr.h
	gcc -c buffer_mgr.c -o buffer_mgr.o

buffer_mgr_stat.o : buffer_mgr_stat.c buffer_mgr_stat.h
	gcc -c buffer_mgr_stat.c -o buffer_mgr_stat.o

test_assign2_1.o : test_assign2_1.c test_helper.h
	gcc -c test_assign2_1.c -o test_assign2_1.o

clean:
	rm -rf *.o 525AssMF