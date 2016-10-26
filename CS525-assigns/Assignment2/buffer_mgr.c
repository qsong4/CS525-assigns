#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"

#define MAX_PAGES 20000
#define MAX_FRAMES 200
#define MAX_K 10
/**
 *  A struct of page frame contain the information of one page of buffer pool
 */
typedef struct frameNode{
    int pageNum;
    int frameNum;
    int dirtyMark;
    int fixCount;
    char* data;
    struct frameNode *next;
    struct frameNode *previous;
}frameNode;

/**
 *  A frame list struct
 */
typedef struct queue{
    frameNode *head;
    frameNode *tail;
}queue;

/**
 *  A struct descript the information of buffer pool
 */

typedef struct bufferInfo{
    int frameNumInBuffer;
    int readTimes;
    int pageToFrame[MAX_PAGES];
    int frameToPage[MAX_FRAMES];
    int writeTimes;
    void *stratData;
    bool dirtyFlags[MAX_FRAMES];
    int fixedCounts[MAX_FRAMES];
    queue *frames;
}bufferInfo;

/**
 *  Initial a new node
 *
 *  @return new node
 */
frameNode *initNode(){
    frameNode *node = malloc(sizeof(frameNode));

    node->data = calloc(PAGE_SIZE, sizeof(SM_FileHandle));
    node->frameNum = 0;
    node->next = NULL;
    node->previous = NULL;
    node->pageNum = NO_PAGE;
    node->dirtyMark = 0;
    node->fixCount = 0;
    return node;
}


/**
 *  Update the Tail of list
 *
 *  @param list    A instance of queue struct
 *  @param updateNode The node update to the tail of queue
 *
 *  @return Null
 */
void enQueue(queue **list, frameNode *updateNode){
    
    frameNode *tail = (*list)->tail;
    frameNode *head = (*list)->head;
    
    if(updateNode == head){

        frameNode *temp = head->next;
        temp->previous = NULL;
        (*list)->head =temp;
        
        updateNode->previous = tail;
        tail->next = updateNode;
        updateNode->next =  NULL;
        
        (*list)->tail=updateNode;
        return;
    }
    else if(updateNode  != tail && updateNode !=head){
        
        updateNode->previous->next = updateNode->next;
        updateNode->next->previous = updateNode->previous;
        
        updateNode->previous = tail;
        tail->next = updateNode;
        updateNode->next =  NULL;
        
        (*list)->tail=updateNode;
        
        return;
    }
    else{
        return;
    }

}


/**
 *  Find the node with given page number
 *
 *  @param list    queue
 *  @param pageNum The number of page
 *
 *  @return NULL
 */
frameNode *findNodewithPageNum(queue *list, const PageNumber pageNum){
    frameNode *current = list->head;
    
    do {
        if(current->pageNum == pageNum){
            return current;
        }
        current = current->next;
    } while (current != NULL);
    
    return NULL;
}
/**
 *  Check if the page in memory, if it is, then fixCount add 1.
 *
 *  @param buffer
 *  @param page
 *  @param pageNum
 *
 *  @return <#return value description#>
 */
frameNode *pageInMemo(BM_BufferPool *const buffer, BM_PageHandle *const page, const PageNumber pageNum){
    
    frameNode *found;
    bufferInfo *info = (bufferInfo *)buffer->mgmtData;
    
    found = findNodewithPageNum(info->frames, pageNum);
    
    if (found!=NULL) {
        page->pageNum = pageNum;
        page->data = found->data;
        
        found->fixCount++;
        
        return found;
    }
    return NULL;
    
}
/**
 *  Update the information of frame node
 *
 *  @param buffer  An instance of BM_bufferPool
 *  @param found   The frame need to update
 *  @param page    An instence of BM_pageHandle
 *  @param pageNum The page number
 *
 *  @return The status
 */
RC updateFrame(BM_BufferPool *const buffer, frameNode *found, BM_PageHandle *const page, const PageNumber pageNum){
    SM_FileHandle fHandle;
    bufferInfo *info = (bufferInfo *)buffer->mgmtData;
    
    RC status;
    status = openPageFile ((char *)(buffer->pageFile), &fHandle);
    if (status != RC_OK){
        return status;
    }

    
    if(found->dirtyMark ==1){
        if((status = ensureCapacity(pageNum, &fHandle)) != RC_OK || (status = writeBlock(found->pageNum, &fHandle, found->data))!= RC_OK){
            return status;
        }
        (info->writeTimes)++;
        (info->readTimes)++;
    }
    (info->pageToFrame)[found->pageNum] = NO_PAGE;
    
    status = ensureCapacity(pageNum, &fHandle);
    if(status != RC_OK){
        return status;
    }
    status = readBlock(pageNum, &fHandle, found->data);
    if(status != RC_OK){
        return status;
    }


    page->pageNum = pageNum;
    page->data = found->data;
    
    (info->readTimes)++;
    

    found->dirtyMark = 0;
    found->fixCount = 1;
    found->pageNum = pageNum;
    
    (info->pageToFrame)[found->pageNum] = found->frameNum;
    (info->frameToPage)[found->frameNum] = found->pageNum;
    
    closePageFile(&fHandle);
    
    return RC_OK;
    
}


/**
 Initial the buffer pool
 
 - returns: Return the status
 */
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData)
{
    int i;
    SM_FileHandle fh;
    bufferInfo *bminfo = malloc(sizeof(bufferInfo));
    
    RC status;
    status = openPageFile ((char *)pageFileName, &fh);
    if (status != RC_OK){
        return status;
    }

    
    bm->numPages = numPages;
    bm->pageFile = (char*) pageFileName;
    bm->strategy = strategy;
    bm->mgmtData = bminfo;
    
    bminfo->frameNumInBuffer = 0;
    bminfo->readTimes = 0;
    bminfo->writeTimes = 0;
    bminfo->stratData = stratData;
    
    memset(bminfo->frameToPage,NO_PAGE,MAX_FRAMES*sizeof(int));
    memset(bminfo->pageToFrame,NO_PAGE,MAX_PAGES*sizeof(int));
    memset(bminfo->dirtyFlags,NO_PAGE,MAX_FRAMES*sizeof(bool));
    memset(bminfo->fixedCounts,NO_PAGE,MAX_FRAMES*sizeof(int));
    
    bminfo->frames = malloc(sizeof(queue));
    queue *frameList = bminfo->frames;
    frameList->head = initNode();
    frameList->tail = initNode();
    frameList->head = frameList->tail;
    
    i = 1;

    while(i<numPages){
        frameList->tail->next = initNode();
        frameList->tail->next->previous = frameList->tail;
        frameList->tail = frameList->tail->next;
        frameList->tail->frameNum = i;
        i++;
    }
    

    
    closePageFile(&fh);
    
    return RC_OK;
}
/**
 *  Shut down the buffer poll
 *
 *  @param bm The point pointer to the buffer pool need to shut down
 *
 *  @return the status
 */
RC shutdownBufferPool(BM_BufferPool *const bm)
{

    if (bm && bm->numPages > 0) {
        RC status;
        status = forceFlushPool(bm);
        if(status != RC_OK){
            
            bufferInfo *bminfo = (bufferInfo *)bm->mgmtData;
            queue *frameList=bminfo->frames;
            frameNode *current = frameList->head;
            
            do{
                current = current->next;
                free(frameList->head->data);
                free(frameList->head);
                frameList->head = current;
            }while(current != NULL);
            
            frameList->head =NULL;
            frameList->tail = NULL;
            free(bminfo->frames);
            free(bminfo);
            
            bm->numPages = 0;
            
            return RC_OK;
            
        }
        else{
            return status;
        }
        

    }
    else{
        return RC_INVALID_BM;
    }

}

/**
 *  write the dirty page back to disk
 *
 *  @param bm A pointer point to bufferpool
 *
 *  @return The status
 */

RC forceFlushPool(BM_BufferPool *const bm)
{
    if (bm && bm->numPages > 0){
        
        bufferInfo *bminfo = (bufferInfo *)bm->mgmtData;
        frameNode *current = bminfo->frames->head;
        
        SM_FileHandle fh;
        RC status;
        status =openPageFile ((char *)(bm->pageFile), &fh);
        if ( status!= RC_OK){
            return status;
        }
        
        do{
            if(current->dirtyMark == 1){
                RC status;
                status = writeBlock(current->pageNum, &fh, current->data);
                if( status == RC_OK){
                    
                    current->dirtyMark = 0;
                    (bminfo->writeTimes)++;
                    (bminfo->readTimes)++;
                    
                }
                else{
                    return RC_WRITE_FAILED;
                }

            }
            current = current->next;
        }while(current != NULL);
        
        closePageFile(&fh);
        
        return RC_OK;
        
    }
    else{
        return RC_INVALID_BM;
    }
    

}

/**
 *  mark the page writed by user dirty
 *
 *  @param bm   The buffer pool
 *  @param page <#page description#>
 *
 *  @return return the status
 */

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (bm && bm->numPages > 0){
        
        bufferInfo *bminfo = (bufferInfo *)bm->mgmtData;
        frameNode *found;
        
        /* Locate the page to be marked as dirty.*/
        found = findNodewithPageNum(bminfo->frames, page->pageNum);
        if(found == NULL){
            return RC_NON_EXISTING_PAGE_IN_FRAME;
        }
        
        /* Mark the page as dirty */
        found->dirtyMark = 1;
        
        return RC_OK;
        

    }
    else{
        
        return RC_INVALID_BM;
    }
    

    
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (bm && bm->numPages > 0){
        
        bufferInfo *bminfo = (bufferInfo *)bm->mgmtData;
        frameNode *found;
        

        found = findNodewithPageNum(bminfo->frames, page->pageNum);
        
        if(found != NULL){
            
            //unpinPage, so decrease the fixcount.
            if(found->fixCount > 0){
                found->fixCount--;
                
            }
            else{
                return RC_NON_EXISTING_PAGE_IN_FRAME;
            }
            
            return RC_OK;
            
            
        }
        else{
            return RC_NON_EXISTING_PAGE_IN_FRAME;
        
        }

        

    }
    else{
        return RC_INVALID_BM;
    }
    

}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)

{
    if (bm && bm->numPages > 0){
        
        bufferInfo *bminfo = (bufferInfo *)bm->mgmtData;
        frameNode *found;
        SM_FileHandle fh;
        RC status;
        status =openPageFile ((char *)(bm->pageFile), &fh);
        if ( status!= RC_OK){
            return RC_FILE_NOT_FOUND;
        }
        
        /* Locate the page to be forced on the disk */
        found = findNodewithPageNum(bminfo->frames, page->pageNum);
        if(found != NULL){
            
            RC status;
            status =writeBlock(found->pageNum, &fh, found->data);
            
            if( status == RC_OK){
                
                (bminfo->writeTimes)++;
                
                closePageFile(&fh);
                
                return  RC_OK;

            }
            closePageFile(&fh);
            return RC_WRITE_FAILED;
            
        }
        else{
            closePageFile(&fh);
            return RC_NON_EXISTING_PAGE_IN_FRAME;
        }

    }
    else{
        return RC_INVALID_BM;
    
    }
    

}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
            const PageNumber pageNum)
{
    RC status;
    frameNode *target;
    bufferInfo *bminfo = (bufferInfo *)bm->mgmtData;
    target = pageInMemo(bm, page, pageNum);
    
    int currentNumFrame = bminfo->frameNumInBuffer;
    int totalNumFrame = bm->numPages;
    
    if (!bm || bm->numPages <= 0){
        return RC_INVALID_BM;
    }
    if(pageNum < 0){
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    switch (bm->strategy)
    {
        case RS_FIFO:
   
            if(target != NULL){
                
                return RC_OK;
            }

            if (currentNumFrame == totalNumFrame) {
                
                target =bminfo->frames->head;
                while (target->fixCount != 0) {
                    target = target->next;
                }
                
                if (target == NULL){
                    return RC_NO_MORE_SPACE_IN_BUFFER;
                }
                
                enQueue(&(bminfo->frames), target);
            }
            else{
                target = bminfo->frames->head;
                int i = 0;
                while(i <currentNumFrame){
                    target = target->next;
                    ++i;
                }
                (bminfo->frameNumInBuffer)++;
            }
            
            status = updateFrame(bm, target, page, pageNum);
            if(status != RC_OK){
                return status;
            }
            
            return RC_OK;
            
            break;
        case RS_LRU:
            
            if(target != NULL){
                enQueue(&(bminfo->frames), target);
                (bminfo->readTimes)++;
                return RC_OK;
            }
            
            if(currentNumFrame == totalNumFrame){
                
                
                target = bminfo->frames->head;  //从head往tail找
                
                while(target->fixCount != 0){
                    target = target->next;//next
                }
                
                if (target == NULL){
                    return RC_NO_MORE_SPACE_IN_BUFFER;
                }
                enQueue(&(bminfo->frames), target);
            }
            else{
                
                target = bminfo->frames->head;
                
                int i = 0;
                while(i < currentNumFrame){
                    target = target->next;
                    ++i;
                }
                (bminfo->frameNumInBuffer)++;
            }
            
            RC status;
            
            if((status = updateFrame(bm, target, page, pageNum)) != RC_OK){
                return status;
            }
            
            
            return RC_OK;
            
            break;
        default:
            return RC_UNKNOWN_STRATEGY;
            break;
    }
    return RC_OK;
}



PageNumber *getFrameContents (BM_BufferPool *const bm)
{
    bufferInfo *bminfo = (bufferInfo *)bm->mgmtData;
    return bminfo->frameToPage;
}

bool *getDirtyFlags (BM_BufferPool *const bm)
{

    bufferInfo *bminfo = bm->mgmtData;
    frameNode *head =bminfo->frames->head;
    frameNode *current = head;
    
    
    do{
        int n = current->frameNum;
        (bminfo->dirtyFlags)[n] = current->dirtyMark;
        current = current->next;
        
    }while (current != NULL);
    
    return bminfo->dirtyFlags;
}

int *getFixCounts (BM_BufferPool *const bm)
{

    bufferInfo *bminfo = bm->mgmtData;
    frameNode *head =bminfo->frames->head;
    frameNode *current = head;
    
    do{
        
        int n = current->frameNum;
        (bminfo->fixedCounts)[n] = current->fixCount;
        current = current->next;
        
    }while (current != NULL);
    
    return bminfo->fixedCounts;
}

int getNumReadIO (BM_BufferPool *const bm)
{
    bufferInfo *bminfo = bm->mgmtData;

    return bminfo->readTimes;
}

int getNumWriteIO (BM_BufferPool *const bm)
{
    bufferInfo *bminfo = bm->mgmtData;
    return bminfo->writeTimes;
}

