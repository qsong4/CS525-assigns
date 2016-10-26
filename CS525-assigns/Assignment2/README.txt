
=========================
#   Problem Statement   #
=========================

The goal of this assignment is to create a buffer manager. The buffer pool has fix size, and we need
to set the replacement strategy.


=========================
# Personal Information #
=========================
NAME                   CWID                 Email
Qingyuan Song       A20358445               qsong4@hawk.iit.edu
Long Cheng          A20362546               lcheng17@hawk.iit.edu
Shengyuan Ye        A20354182               sye7@hawk.iit.edu
Junjie Ying         A20293306               jying@hawk.iit.edu

=========================
# File List #
=========================
1.dberror.c
2.dberror.h
3.storage_mgr.c
4.storage_mgr.h
5.test_assign2_1.c
6.buffer_mgr_stat.c
7.buffer_mgr_stat.h
8.buffer_mgr.c
9.buffer_mgr.h
10.dt.h
11.test_helper.h

=========================
# How To Run The Script #
=========================

There are two test cases.
At the project's directory run following command on terminal:
————————————————————————————
Compile : make
Run : ./525Assignment2_1




To revert:
————————————————————————————
On terminal : make clean



=========================
#  Addtional Function   #
=========================
None

=========================
#  Data Structure   #
=========================
main data structure used

=========================
#  Extra Credit   #
=========================

NONE

==========================
# Additional error codes #
==========================


#define RC_READ_FAIL 100
#define RC_CANNT_SET_POINTER 101
#define RC_FILE_ALREADY_EXIST 102
#define RC_PAGE_OUTOF_RANGE 103
#define RC_NO_SUCH_PAGE_IN_BUFF 104
#define RC_UNESPECTED_ERROR 105

==========================
#    Test Cases       #
==========================

test_assign2_1.c

