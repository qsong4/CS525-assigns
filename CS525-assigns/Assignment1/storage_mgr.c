#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "storage_mgr.h"
#include "dberror.h"

void initStorageManager (void){
}

/*
*******************  Page Functions  *************************
*/
/**
 *  This function use for create the file with name filename
 *
 *  @param fileName This is a pointer param which mean the name of file
 *
 *  @return Return the status
 */

RC createPageFile (char *fileName){

    if(fileName==NULL){
        return RC_FILE_NOT_FOUND;
    }
    //create the file
    FILE *file=fopen(fileName,"w+");
    //fill the block with '/0'
    if(file)
    {
        char firstPage[PAGE_SIZE];
        memset(firstPage,'\0',PAGE_SIZE);
        //The first page of file is zero
        long result=fwrite(firstPage, sizeof(char), PAGE_SIZE, file);
        
        fclose(file);
        return RC_OK;
    }
    
    return RC_FILE_NOT_FOUND;

}

/**
 *  This function use for open a file.
 *
 *  @param fileName This is the name of file
 *  @param fHandle  Save open file's information
 *
 *  @return return the status of function
 */
RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    
    FILE *pagef = fopen(fileName, "r+");

    
    if(pagef){
        int flag = fseek(pagef,0L,SEEK_END);
        long filesize=0;
        filesize = (ftell(pagef)+1)/PAGE_SIZE;
        fHandle->fileName = fileName;
        fHandle->totalNumPages = filesize;
        fHandle->curPagePos = 0;
        fHandle->mgmtInfo = pagef;
        return RC_OK;
    }
    return RC_FILE_NOT_FOUND;
    


}
/**
 *  Close the file
 *
 *  @param fHandle The structure incloud the info of file
 *
 *  @return return the status of function
 */
RC closePageFile (SM_FileHandle *fHandle){
    
    
    if (fclose(fHandle->mgmtInfo)==0) {
        return RC_OK;
    }
    return RC_FILE_NOT_FOUND;

}


/**
 *  destroy the close file
 *
 *  @param fileName The name of file
 *
 *  @return return the status of function
 */
RC destroyPageFile (char *fileName){
  int check = remove(fileName);
  if (!check){
    return RC_OK;
  }

  return RC_FILE_NOT_FOUND;
    
}

/*
*******************  Read Functions  *************************
*/

/**
 *  the readBlock function reads page from the selected file into the memory pointed by SM_PageHandle
 *
 *  @param pageNum indicates the page number user want to read
 *  @param fHandle saves opend file's infomation
 *  @param memPage where page content is saved
 *
 *  @return RC_OK indicates reading success
 */
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    if(fHandle)
    {
        if(pageNum>fHandle->totalNumPages||pageNum<0)
        {
            return RC_READ_NON_EXISTING_PAGE;
        }
        
        int setPointer = fseek(fHandle->mgmtInfo, PAGE_SIZE*(pageNum), SEEK_SET);
        if(setPointer==-1){
            return RC_CANNT_SET_POINTER;
        }
        
        size_t flag = fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);
        if(flag == 0){
            return RC_READ_FAIL;
        }
        
        fHandle->curPagePos=pageNum;
        return RC_OK;
    
    }
    return RC_READ_FAIL;

}

/**
 *  find the current page position in a file
 *
 *  @param fHandle saves opend txt file's infomation
 *
 *  @return a page number is been reading now
 */

int getBlockPos (SM_FileHandle *fHandle)
{
    return fHandle->curPagePos;
}

/**
 *  the first page content
 *
 *  @param fHandle saves opend txt file's infomation
 *  @param memPage where page content is saved
 *
 *  @return RC_OK indicates reading success
 */
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    return readBlock(0, fHandle, memPage);
}

/**
 *  the previous page content
 *
 *  @param fHandle saves opend txt file's infomation
 *  @param memPage where page content is saved
 *
 *  @return RC_OK indicates reading success
 */
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    return readBlock(fHandle->curPagePos-1, fHandle, memPage);
}
/**
 *  the current page content
 *
 *  @param fHandle saves opend txt file's infomation
 *  @param memPage where page content is saved
 *
 *  @return RC_OK indicates reading success
 */
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}
/**
 *  the next page content
 *
 *  @param fHandle saves opend txt file's infomation
 *  @param memPage where page content is saved
 *
 *  @return RC_OK indicates reading success
 */
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    return readBlock(fHandle->curPagePos+1, fHandle, memPage);
}
/**
 *  the last page content
 *
 *  @param fHandle saves opend txt file's infomation
 *  @param memPage where page content is saved
 *
 *  @return RC_OK indicates reading success
 */
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    return readBlock(fHandle->totalNumPages, fHandle, memPage);
}


/*
*******************  Write Functions  *************************
*/

/**
 *  Description:
 *              Write a block in memory to file
 *
 *  @param pageNum which page do you want to be written
 *  @param fHandle The structure incloud the info of file
 *  @param memPage The pointer points the data in memory
 *
 *  @return success or fail
 */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    
    //make sure pageNum is valid
    if(pageNum > fHandle->totalNumPages || pageNum < 0){
        return RC_FILE_NOT_FOUND;
    }
    //set the file pointer
    long size = (pageNum)*PAGE_SIZE*sizeof(char);
    int seekFlg = fseek(fHandle->mgmtInfo, size, SEEK_SET);
    
    if (seekFlg == 0) {
        //make sure 
        fwrite(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);
        fHandle->curPagePos = pageNum;
        return RC_OK;
        }
    
    else{
        return RC_CANNT_SET_POINTER;
    }
    

}

/**
 *  Write current block in memory to a file
 *
 *  @param fHandle The structure incloud the info of file
 *  @param memPage The pointer points the data in memory
 *
 *  @return success or fail
 */
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Make sure the file is exist
    if (fHandle->mgmtInfo == NULL) {
        return RC_FILE_NOT_FOUND;
    }
    //call writeBlock write the block in a file
	else return writeBlock (fHandle->curPagePos, fHandle, memPage);
}

/**
 *  Increase the number of pages in the file by one. The new last page should be filled with zero bytes.
 *
 *  @param fHandle The structure incloud the info of file
 *
 *  @return success or fail
 */
RC appendEmptyBlock (SM_FileHandle *fHandle){
	
    int seekFlg;
    
    //allocates memory and return a pointer to it
    char *newPage = calloc(PAGE_SIZE, sizeof(char));
    //seeks file write pointer to the last page
    seekFlg = fseek(fHandle->mgmtInfo,(fHandle->totalNumPages)*PAGE_SIZE*sizeof(char) , SEEK_END);
    
    if (seekFlg == 0){
        
        // writes data from the memory block pointed by newPage to the file
       size_t writeSize = fwrite(newPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);
        if (writeSize==0)
        {
            RC_WRITE_FAILED;
        }
        
        fHandle->totalNumPages = fHandle->totalNumPages + 1;
        fHandle->curPagePos = fHandle->totalNumPages;
        rewind(fHandle->mgmtInfo);

        free(newPage);
        return RC_OK;
	}
    else{
        free(newPage);
        return RC_WRITE_FAILED;
	}
}
/**
 *  If the file has less than numberOfPages pages then increase the size to numberOfPages.
 *
 *  @param numberOfPages The number of pages the file should have
 *  @param fHandle       The structure incloud the information of file
 *
 *  @return success or fail
 */
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    //How many pages we need to add to meet the numberOfPages
    int pages;
    int i;
    int totalNum = fHandle->totalNumPages;
    // Use appendOfPages to increase the number of pages in file
	if (totalNum < numberOfPages)
        {

		pages = numberOfPages - totalNum;

        for (i=0; i < pages; i++){
			appendEmptyBlock(fHandle);
        }
        }
    return RC_OK;
}
