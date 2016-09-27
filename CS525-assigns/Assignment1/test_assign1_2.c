#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"

/* prototypes for test functions */
static void testSinglePageContent(void);

/* main function running all tests */
int
main (void)
{
    testName = "";
    
    initStorageManager();

    testSinglePageContent();
    
    return 0;
}


/* check a return code. If it is not RC_OK then output a message, error description, and exit */
/* Try to create, open, and close a page file */

void
testSinglePageContent(void)
{
    SM_FileHandle fh;
    SM_PageHandle ph,ph2;
    int i;
    
    testName = "test single page content";
    
    ph = (SM_PageHandle) malloc(PAGE_SIZE);
    ph2 =(SM_PageHandle) malloc(PAGE_SIZE);
    
    // create a new page file
    TEST_CHECK(createPageFile (TESTPF));
    TEST_CHECK(openPageFile (TESTPF, &fh));
    printf("created and opened file\n");
    
    TEST_CHECK(appendEmptyBlock(&fh));
    ASSERT_TRUE((fh.totalNumPages == 2), "expect 2 pages after append an empty page");
    
    // read current page into handle
    TEST_CHECK(readCurrentBlock (&fh, ph));
    // the page should be empty (zero bytes)
    for (i=0; i < PAGE_SIZE; i++)
        ASSERT_TRUE((ph[i] == 0), "expected zero byte in appending page ");
    printf("appending page was empty\n");
    
    // change ph to be a string and write that one to appended page
    for (i=0; i < PAGE_SIZE; i++)
        ph[i] = (i % 10) + '0';
    TEST_CHECK(writeCurrentBlock (&fh, ph));
    printf("writing appended block\n");
    
    // read back the page containing the string and check that it is correct
    TEST_CHECK(readCurrentBlock (&fh, ph));
    for (i=0; i < PAGE_SIZE; i++)
        ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
    printf("reading appended block\n");
    
    // increase the capacity of file to 3.
    TEST_CHECK(ensureCapacity (3, &fh));
    printf("Total number of pages:%d\n",fh.totalNumPages);
    ASSERT_TRUE((fh.totalNumPages == 3), "expect 3 pages after increase the capacity");
    
    // change ph to be a string and write that one to appended page
    for (i=0; i < PAGE_SIZE; i++)
        ph2[i] = 'a';
    TEST_CHECK(writeCurrentBlock (&fh, ph2));
    printf("writing appended block with 'a'\n");
    
    // read previous page
    TEST_CHECK(readPreviousBlock (&fh, ph));
    for (i=0; i < PAGE_SIZE; i++)
        ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected");
    printf("reading the previous block\n");
    
    // read next page into handle
    TEST_CHECK(readNextBlock (&fh, ph));
    // the page should be empty (zero bytes)
    for (i=0; i < PAGE_SIZE; i++)
        ASSERT_TRUE((ph[i] == 'a'), "expected 'a' in lastly appened page");
    printf("reading the next block which was appended earlier. Expected 'a'\n");
    
    
    // read last page into handle
    TEST_CHECK(readLastBlock (&fh, ph));
    // the page should be empty (zero bytes)
    for (i=0; i < PAGE_SIZE; i++)
        ASSERT_TRUE((ph[i] == 'a'), "expected 'a' in newly appened page");
    printf("reading the last block which was appended earlier. Expected 'a'\n");


    
    // destroy new page file
    TEST_CHECK(destroyPageFile (TESTPF));
    
    TEST_DONE();
}
