all:525Assignment1_1 525Assignment1_2

525Assignment1_1 : dberror.o storage_mgr.o test_assign1_1.o
	gcc dberror.o storage_mgr.o test_assign1_1.o -o 525Assignment1_1

525Assignment1_2 : dberror.o storage_mgr.o test_assign1_2.o
	gcc dberror.o storage_mgr.o test_assign1_2.o -o 525Assignment1_2

dberror.o: dberror.c dberror.h
	gcc -o dberror.o -c dberror.c

storage_mgr.o: storage_mgr.c storage_mgr.h
	gcc -o storage_mgr.o -c storage_mgr.c

test_assign1_1.o: test_assign1_1.c test_helper.h
	gcc -o test_assign1_1.o -c test_assign1_1.c

test_assign1_2.o: test_assign1_2.c test_helper.h
	gcc -o test_assign1_2.o -c test_assign1_2.c

clean:
	rm -rf *.o 525Assignment1 525Assignment1_2
