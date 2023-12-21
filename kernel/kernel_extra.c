/*
 * kernel_extra.c - Project 2 Extra Credit, CMPSC 473
 * Copyright 2023 Ruslan Nikolaev <rnikola@psu.edu>
 * Distribution, modification, or usage without explicit author's permission
 * is not allowed.
 */

#include <malloc.h>
#include <types.h>
#include <string.h>
#include <printf.h>

// Your mm_init(), malloc(), free() code from mm.c here
// You can only use mem_sbrk(), mem_heap_lo(), mem_heap_hi() and
// Project 2's kernel headers provided in 'include' such
// as memset and memcpy.
// No other files from Project 1 are allowed!

typedef void* blk_ptr;

static inline int WSIZE(void) {
    return 8;
}

static inline int DSIZE(void) {
    return 16;
}

//this number was chosen because the book also used it previously it was left shifting, but then i changed it to return integer, to save the left shift.
static inline int CHUNKSIZE(void){
    return 4096;
}

/* What is the correct alignment? */
#define ALIGNMENT 16

/* rounds up to the nearest multiple of ALIGNMENT */
static inline size_t align(size_t x){
    return ALIGNMENT * ((x+ALIGNMENT-1)/ALIGNMENT);
}

//made this up because i was using 2*DSIZE() every where which i thought could cause a error, so this
static inline size_t MIN_BLOCK_SIZE(void){
    return (size_t)2*DSIZE();
}
// compute the maximum of 2 values
static inline size_t MAX(size_t x, size_t y){ 
    return ((x > y) ? (x) : (y));
}

//compute the minimum of the 2 values
static inline size_t MIN(size_t x, size_t y){ 
    return ((x < y) ? (x) : (y));
}

// pack the size and the allocated bit into a word
static inline size_t PACK(size_t size, int alloc){
    return ((size) | (alloc));
}

// read and write at the address of bp
static inline size_t GET(blk_ptr bp){
    return (*(size_t *)(bp));
}

//write the address of bp
static inline void PUT(blk_ptr bp, size_t val){
    *((size_t *)(bp)) = val;
}

// put the ptr in the addresss pointed by bp, to make a doubly linked list
static inline void PUT_PTR(blk_ptr bp, blk_ptr ptr){ 
    *(size_t *)(bp) = (size_t)(ptr);
}

// return the size of block 
static inline size_t GET_SIZE(blk_ptr bp){
    return (size_t)(GET(bp) & ~(0x7));
}

// return the allocated bit
static inline size_t GET_ALLOC(blk_ptr bp){
    return (size_t)(GET(bp) & 0x1);
}

// compute the address of header and footer
static inline size_t *HDRP(void *bp){
    return ((size_t *)(bp) - 1);
}

//compute the address of footer
static inline size_t *FTRP(void *bp){
    return ((size_t *)((bp) + GET_SIZE(HDRP(bp)) - 16));
}

// compute the address of previous and next block given bp
static inline size_t *PREV_BLKP(blk_ptr bp){
    return (size_t *)((bp) - GET_SIZE((bp) - 16));
}

static inline size_t *NEXT_BLKP(blk_ptr bp){
    return (size_t *)((bp) + GET_SIZE(HDRP(bp)));
}

// compute the address of place where the previous or next pointer is to be stored or is stored in the block pointed to by bp
static inline size_t *PREV_PTR_IN_BLK(blk_ptr bp){
    return ((size_t *)((bp)));
}

static inline size_t *NEXT_PTR_IN_BLK(blk_ptr bp){
    return ((size_t *)((bp)+8));
}

// compute the address of previous and next block in the doubly linked list
static inline size_t *PREV_LIST_BLKP(blk_ptr bp){
    return (*(size_t **)(bp));
}
static inline size_t *NEXT_LIST_BLKP(blk_ptr bp){
    return (*(size_t **)(NEXT_PTR_IN_BLK(bp)));
}

// declare segregated list array and a pointer that points to beginning of heap, used only in mm_init to make the prologue and epilogue header and footer
blk_ptr SEGLIST_ARR[14];
blk_ptr heap_listp = NULL;

//declared all of this here so i could get rid of the implicit functions warning
static blk_ptr extend_heap(size_t size);
static blk_ptr coalesce(void *ptr);
static blk_ptr place(void* ptr, size_t asize);
static void REMOVE_FROM_LIST(void *ptr);
static void ADD_TO_LIST(void *ptr, size_t size);
static int SEARCH_INDEX_IN_SEGLIST(size_t asize);

// function that extends the heap and create free blocks
static blk_ptr extend_heap(size_t wsize){
	size_t asize = align(wsize);
	size_t *bp;
	
	if((size_t *)(bp = mem_sbrk(asize)) == (void *)-1){
		return NULL;
	}
	
	// add to segregated list, doing this first so nothing of importance is changed later or in any call after in the function, was getting random errors
	ADD_TO_LIST(bp,asize);
	
	// set free block header and footer and epilogue header
	PUT(HDRP(bp), PACK(asize, 0));
	PUT(FTRP(bp), PACK(asize, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));
	return coalesce(bp);
}

// function that combines adjacent free blocks into one larger free block
// and remove the appropriate free blocks from the list
//right now this function coalesces every free block, even if it needs to be coalesced or not
static blk_ptr coalesce(void *bp){
	
    //Determine if Previous and Next Blocks are Allocated:
    // blk_ptr prev_blk = PREV_BLKP(bp);
    // assert("");
    // blk_ptr prev_blk_ftrp = FTRP(prev_blk);
    size_t prev_alloc =    GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t prev_size = GET_SIZE(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));
    size_t current_size = GET_SIZE(HDRP(bp));
    size_t cu_size;
    // size_t index1 = SEARCH_INDEX_IN_SEGLIST(current_size);
    // size_t index2 = SEARCH_INDEX_IN_SEGLIST(next_size);
    // size_t index3 = SEARCH_INDEX_IN_SEGLIST(prev_size);
    


    /*NEEDFUL = CHECK IF THE BLOCK IS ALREADY IN SEG LIST, IF YES THEN REMOVE IT, UPDATE THE LIST, AND THEN ADD POINTERS AFTER COALESCE
    The POINTS we have are:
    1. BASIC CHECKS FOR ALLOCATION ARE NECESSARY, THEY FORM THE BEDROCK OF COALESCE
    2. AFTER BASIC CHECKS WE HAVE OPTIONS
        2.1 THE CURRENT SIZE FITS IN SEG LIST CLASS AND THE FREE BLOCK SIZE DOES THAT TOO, THEIR ADDITION ALSO FITS, THEN WE COALESCE AND DO THE NEEDFUL
        2.2 CURRENT SIZE AND FREE BLOCK SIZE BOTH MATCH, ADDITION DOES NOT, DONT COALESCE, NO NEED
        2.3 CURRENT SIZE FITS, FREE BLOCK SIZE DOESNT, OR VICE VERSA, IF ADDITION FITS, COALESCE AND DO THE NEEDFUL, IF IT DOES NOT FIT, THEN ADD THE ONE THAT DOESNT FIT TO THE LIST OF BLOCKS THAT WE ARE MAINTAINING, AND THE REMAINING ONE, ADD IT TO SEG LIST
        2.4 CURRENT SIZE AND FREE BLOCK SIZE BOTH DONT FIT, ADDITION DOES, THEN COALESCE AND DO THE NEEDFUL, AND IF THE ADDITION DONT FIT, STILL COALESCE*/
    
   //Case 1: Both Previous and Next Blocks are Allocated: 
    if (prev_alloc && next_alloc) {
        //use the insert in linked list function for the rest
        return bp;
    }

    //Case 2: Previous Block is Allocated, Next Block is Free:
    else if (prev_alloc && !next_alloc) {
        cu_size = current_size + next_size;
        
        REMOVE_FROM_LIST(bp);
        REMOVE_FROM_LIST(NEXT_BLKP(bp));

        PUT(HDRP(bp),PACK(cu_size,0));
        PUT(FTRP(bp), PACK(cu_size,0));       
        
    } 
    //Case 3: Previous Block is Free, Next Block is Allocated:
    else if (!prev_alloc && next_alloc) {
        cu_size = current_size + prev_size;

        REMOVE_FROM_LIST(bp);
        REMOVE_FROM_LIST(PREV_BLKP(bp));

        PUT(FTRP(bp), PACK(cu_size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(cu_size, 0));
        bp = PREV_BLKP(bp);
        
    }
    //Case 4: Both Previous and Next Blocks are Free:  
    else {
        cu_size =current_size + next_size+prev_size;
        
        REMOVE_FROM_LIST(bp);
        REMOVE_FROM_LIST(NEXT_BLKP(bp));
        REMOVE_FROM_LIST(PREV_BLKP(bp));

        PUT(FTRP(NEXT_BLKP(bp)), PACK(cu_size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(cu_size, 0));
        bp = PREV_BLKP(bp);
            
    }

    ADD_TO_LIST(bp,cu_size);
    return bp;
}

// function that search the segregated list to find the appropriate size class
// and return the size class index of the segregated list
//using while loop, it returns index when the value reaches a certain threshold
static int SEARCH_INDEX_IN_SEGLIST(size_t size){
	
    size_t classSize = 32;
    int index = 1;
    while (classSize < 131074)
    {
        if (size < 32)
        {
            return 0;
        }
        
        if(size == 32){
            return 0;
        }
        else if(size>=classSize && size<(classSize*2)){
            return index;
        }
        classSize=classSize*2;
        index++;
        // printf("index : %d \n", index);

        if(index >=14 ){
            return 13;
        }
    }
    return -1;
}

// function that place the requested block into free block
// compute the remainning size of the free block, if it is less 
// or equal to the min free block size, then allocate free block
// and add to the list.
static blk_ptr place(blk_ptr bp, size_t newSize){

    REMOVE_FROM_LIST(bp);//we will remove the block first before performing any operations on it, so it doesnot in any way shape or form hurt out seg list array
    size_t cSize ;
    size_t* n;
    cSize = GET_SIZE(HDRP(bp));
    // int index  = SEARCH_INDEX_IN_SEGLIST(cSize);
    // REMOVE_FROM_LIST(bp);

    if ((cSize - newSize) >= (size_t)(2*DSIZE())) {
        //if there is room to be spliced, preform splicing.
        PUT(HDRP(bp), PACK(newSize, 1));
        PUT(FTRP(bp), PACK(newSize, 1));
        n = NEXT_BLKP(bp);
        //where do i have to keep track of this newly freed block?????
        PUT(HDRP(n), PACK(cSize-newSize, 0));
        PUT(FTRP(n), PACK(cSize-newSize, 0));
        ADD_TO_LIST(n,cSize-newSize);//THIS IS NOT COMPLETE, SOME FUNCTIONS AND EDGE CASES NEED TO BE TAKEN CARE OF
	    return bp;

    }
    else {
        //if not needed to be spliced, allocate the whole block.
        PUT(HDRP(bp), PACK(cSize, 1));
        PUT(FTRP(bp), PACK(cSize, 1));
        n = NEXT_BLKP(bp);
        if(!GET_ALLOC(HDRP(n))){
            PUT(FTRP(n),GET(HDRP(n)));
        }
	    return bp;
    }
}

// function that remove the block from the segregated list given
// pointer and fix the the pointers
static void REMOVE_FROM_LIST(blk_ptr bp){
	// get block size info
	
    size_t size = GET_SIZE(HDRP(bp));

    //search for the size class
    int index = SEARCH_INDEX_IN_SEGLIST(size);

    if(PREV_LIST_BLKP(bp) == NULL){//if there is no previous block means its at the head
        if(NEXT_LIST_BLKP(bp)!=NULL){//if there is no next block means its the only block
            PUT_PTR(PREV_PTR_IN_BLK(NEXT_LIST_BLKP(bp)),NULL);
            SEGLIST_ARR[index] = NEXT_LIST_BLKP(bp);

        }
        else{
            SEGLIST_ARR[index] = NULL;//since the only block just take it and make the value in array for the size class NULL, for further operations
        }
    }
    else{
        if(NEXT_LIST_BLKP(bp)!=NULL){//if next is null means end of list
            PUT_PTR(PREV_PTR_IN_BLK(NEXT_LIST_BLKP(bp)),PREV_LIST_BLKP(bp));
            PUT_PTR(NEXT_PTR_IN_BLK(PREV_LIST_BLKP(bp)), NEXT_LIST_BLKP(bp));
            
        }
        else{
            PUT_PTR(NEXT_PTR_IN_BLK(PREV_LIST_BLKP(bp)),NULL);//end of list only the previous is updated and made NULL
        }
    }
}

// function that insert the free block into segregated list
static void ADD_TO_LIST(blk_ptr bp, size_t size){
 	// find the appropirate size class list

    int index = SEARCH_INDEX_IN_SEGLIST(size);
    blk_ptr headP = SEGLIST_ARR[index];

    if (headP != NULL)//just adding to the head always, it increases fragmentation but i think that looping through the linked list to add a block will decrease throughput
    {
        SEGLIST_ARR[index] = bp;
        PUT_PTR(PREV_PTR_IN_BLK(bp),NULL); //making the prev in the current block null
        PUT_PTR(NEXT_PTR_IN_BLK(bp), headP);// making next in current block point to the next block
        PUT_PTR(PREV_PTR_IN_BLK(headP), bp);//making the previous in next block point to the current block
    }
    else{//if head pointer is NULL, then it is the first block
        SEGLIST_ARR[index] = bp;
        PUT_PTR(PREV_PTR_IN_BLK(bp),NULL);// making prev and next null since its the first block, can point to nothing
        PUT_PTR(NEXT_PTR_IN_BLK(bp), NULL);
    }
    
}





bool mm_init()
{	
    for (int i =0; i <14; i++){
        SEGLIST_ARR[i]=NULL;
    }
    
    if ((heap_listp = mem_sbrk(4*WSIZE())) == (void *)-1)
        return false;
    PUT(heap_listp, 0); /* Alignment padding */
    PUT(heap_listp + (1*WSIZE()), PACK(DSIZE(), 1)); /* Prologue header */

    // printf("%p\n",heap_listp + (1*WSIZE()));
    PUT(heap_listp + (2*WSIZE()), PACK(DSIZE(), 1)); /* Prologue footer */
    // printf("%p\n",heap_listp + (2*WSIZE()));
    PUT(heap_listp + (3*WSIZE()), PACK(0, 1)); /* Epilogue header */
    // printf("%p\n",heap_listp + (3*WSIZE()));
    heap_listp += (2*WSIZE());
    // printf("DID THE PROLOGUE HDR & FTR & EPILOGUE.\n");
    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    // printf("xtending heap in mm init\n");

    if (extend_heap(CHUNKSIZE()) == NULL)
        return false;
    return true;
    
	// return false;
}

void *malloc(size_t size)
{
     blk_ptr bp = NULL;
    size_t newSize;
    size_t extendSize;
    int index = 0;
    // align the size first, tried to use the alignment function given but was throwing errors no idea why, didnt dive too deep in it
   if(size==0) return NULL;
   if(size<=(size_t)DSIZE()){
    newSize=2*DSIZE();  
    }
    else{
        newSize = ((size + DSIZE() + (DSIZE() - 1 ))/ DSIZE()) * DSIZE();
        
    }
    
    /*what we want to do is get the new size and then search the seg list array with the newsize so that we can find the right index to get to the correct list array index*/
    index = SEARCH_INDEX_IN_SEGLIST(newSize);// we want this to be the index whixh we will get from the function that will search through the seg list
    extendSize = MAX(newSize,CHUNKSIZE());
    //this if is so if the search returns -1 it exits the whole thing
    if(index!= 14){
        bp = SEGLIST_ARR[index];

        if(bp!= NULL){//checks to see if there is something in index, so it sees if there is a pointer to a free block at the place
            for(int i =0; i < 32; i++){//using a for loop, just so that the loop stops after sometime and does not keep finding a block in the linked list if its a very big linked list, i used while but that made the throughput go from 91 to 87, thus, checked a lot of number for 2 its 89, for 4,8 its 90, and 16 to 4096(going in powers of 2), its 91, but the avg throughput maximum was at 32.
            // while (true){
                if(bp == NULL) break;

                if(newSize <= GET_SIZE(HDRP(bp))){
                    bp = place(bp,newSize);
                    return bp;
                }
                bp = NEXT_LIST_BLKP(bp);
                // break;
            }
        }
    }
    else{
        printf("malloc failed");
        return NULL;
    }

    index++;
    bp = NULL;
    

    while ((index < 14) && (bp == NULL)) // this is written because lets say that the index<14, but in 32 iterations we cant find the block we want then we will do this, and search in the next list
    {
        bp = SEGLIST_ARR[index];
        index++;
    }
   

    if (bp == NULL)
    {
        bp = extend_heap(extendSize);
    }


    bp = place(bp, newSize);
    return bp;
    
	return NULL;
}

void free(void *ptr)
{
     if(ptr == NULL) return;
    size_t size = GET_SIZE(HDRP(ptr));
       
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    ADD_TO_LIST(ptr, size);//frees it adds it to list and sends it to coalesce
    coalesce(ptr);
}
