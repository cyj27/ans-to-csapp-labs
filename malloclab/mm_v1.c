/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE       4       
#define DSIZE       8       
#define CHUNKSIZE  (1<<9)  

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* packing the size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* get the size and allocated bit (through reading Header/Footer) */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* get the header and footer address of a block */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* get the address of the previous and next block in physical memory */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* get the address of the previous and next block in the free list */
#define GET_PRED(bp) (*(char **)(bp))
#define GET_SUCC(bp) (*(char **)((char *)(bp) + WSIZE))

#define SET_PRED(bp, val) (*(char **)(bp) = (char *)(val))
#define SET_SUCC(bp, val) (*(char **)((char *)(bp) + WSIZE) = (char *)(val))

/* pointer to the head of the free list */
static char *free_lists[10];
static char *heap_listp;
static int get_list_idx(size_t size);
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void *place(void *bp, size_t asize);
static void insert_node(void *bp);
static void delete_node(void *bp);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 2*WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 3*WSIZE, PACK(0, 1));
    heap_listp += 2*WSIZE;
    for(int i = 0; i < 10; i++){
        free_lists[i] = NULL;
    }    
    //expend the heap
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;
    if (size <= 0)
        return NULL;
    asize = ALIGN(size + SIZE_T_SIZE);
    if ((bp = find_fit(asize)) != NULL) {
        bp = place(bp, asize);
        return bp;
    }
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    bp = place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    size_t oldsize = GET_SIZE(HDRP(ptr));
    size_t newsize = ALIGN(size + SIZE_T_SIZE);
    if (newsize <= oldsize) {
        if (oldsize - newsize >= 2 * DSIZE) {
            PUT(HDRP(ptr), PACK(newsize, 1));
            PUT(FTRP(ptr), PACK(newsize, 1));
            void *next_bp = NEXT_BLKP(ptr);
            PUT(HDRP(next_bp), PACK(oldsize - newsize, 0));
            PUT(FTRP(next_bp), PACK(oldsize - newsize, 0));
            coalesce(next_bp); 
        }
        return ptr;
    }
    
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    
    if (!next_alloc && (oldsize + next_size >= newsize)) {
        size_t total_avail = oldsize + next_size;
        void *next_bp = NEXT_BLKP(ptr);
        delete_node(next_bp); 
        
        if (total_avail - newsize >= 2 * DSIZE) {
            PUT(HDRP(ptr), PACK(newsize, 1));
            PUT(FTRP(ptr), PACK(newsize, 1));
            void *new_next = NEXT_BLKP(ptr);
            PUT(HDRP(new_next), PACK(total_avail - newsize, 0));
            PUT(FTRP(new_next), PACK(total_avail - newsize, 0));
            coalesce(new_next);
        } else {
            PUT(HDRP(ptr), PACK(total_avail, 1));
            PUT(FTRP(ptr), PACK(total_avail, 1));
        }
        return ptr;
    }
    
    _Bool is_epilogue = (next_size == 0);
    _Bool is_before_epilogue = (!next_alloc && GET_SIZE(HDRP(NEXT_BLKP(NEXT_BLKP(ptr)))) == 0);
    
    if (is_epilogue || is_before_epilogue) {
        size_t total_avail = oldsize + next_size;
        
        if (total_avail < newsize) {
            size_t extend_size = newsize - total_avail;
            if (extend_heap(extend_size / WSIZE) == NULL)
                return NULL;
                
            total_avail = oldsize + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        }
        
        void *next_bp = NEXT_BLKP(ptr);
        delete_node(next_bp); 
        
        if (total_avail - newsize >= 2 * DSIZE) {
            PUT(HDRP(ptr), PACK(newsize, 1));
            PUT(FTRP(ptr), PACK(newsize, 1));
            void *new_next = NEXT_BLKP(ptr);
            PUT(HDRP(new_next), PACK(total_avail - newsize, 0));
            PUT(FTRP(new_next), PACK(total_avail - newsize, 0));
            coalesce(new_next);
        } else {
            PUT(HDRP(ptr), PACK(total_avail, 1));
            PUT(FTRP(ptr), PACK(total_avail, 1));
        }
        return ptr;
    }
    
    void *newptr = mm_malloc(size); 
    if (newptr == NULL)
        return NULL;
    memcpy(newptr, ptr, oldsize - DSIZE); 
    mm_free(ptr);
    return newptr;
}
void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        insert_node(bp);
        return bp;
    }
    else if (!prev_alloc && next_alloc) {
        delete_node(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        insert_node(bp);
    }
    else if (prev_alloc && !next_alloc) {
        delete_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        insert_node(bp);
    }
    else {
        delete_node(PREV_BLKP(bp));
        delete_node(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        insert_node(bp);
    }
    return bp;
}

void insert_node(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    int idx = get_list_idx(size);
    void *search_ptr = free_lists[idx];
    void *pred_ptr = NULL;
    while(search_ptr != NULL && GET_SIZE(HDRP(search_ptr)) < size) {
        pred_ptr = search_ptr;
        search_ptr = GET_SUCC(search_ptr);
    }
    if (pred_ptr == NULL && search_ptr == NULL) {
        free_lists[idx] = bp;
        SET_PRED(bp, NULL);
        SET_SUCC(bp, NULL);
    }
    else if (pred_ptr == NULL && search_ptr != NULL) {
        SET_SUCC(bp, search_ptr);
        SET_PRED(bp, NULL);
        SET_PRED(search_ptr, bp);
        free_lists[idx] = bp;
    }else if (pred_ptr != NULL && search_ptr == NULL) {
        SET_SUCC(bp, NULL);
        SET_PRED(bp, pred_ptr);
        SET_SUCC(pred_ptr, bp);
    }else {
        SET_SUCC(bp, search_ptr);
        SET_PRED(bp, pred_ptr);
        SET_SUCC(pred_ptr, bp);
        SET_PRED(search_ptr, bp);
    }
}

void delete_node(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    int idx = get_list_idx(size);
    void *pred = GET_PRED(bp);
    void *succ = GET_SUCC(bp);
    if (pred == NULL && succ == NULL)
        free_lists[idx] = NULL;
    else if (pred == NULL && succ != NULL) {
        SET_PRED(succ, NULL);
        free_lists[idx] = succ;
    }
    else if (pred != NULL && succ == NULL) {
        SET_SUCC(pred, NULL);
    }
    else {
        SET_SUCC(pred, succ);
        SET_PRED(succ, pred);
    }
}

void *find_fit(size_t asize)
{
    int idx = get_list_idx(asize);
    for(int i = idx; i < 10; i++){
        void *bp = free_lists[i];
        while (bp != NULL) {
            if (GET_SIZE(HDRP(bp)) >= asize)
                return bp;
            bp = GET_SUCC(bp);
        }
    }
    return NULL;
}

void *place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    delete_node(bp); 
    if ((csize - asize) >= (2 * DSIZE)) { 
        if (asize >= 96) { 
            PUT(HDRP(bp), PACK(csize - asize, 0));
            PUT(FTRP(bp), PACK(csize - asize, 0));
            insert_node(bp);
            void *next_bp = NEXT_BLKP(bp);
            PUT(HDRP(next_bp), PACK(asize, 1));
            PUT(FTRP(next_bp), PACK(asize, 1));
            return next_bp;
        } else {
            PUT(HDRP(bp), PACK(asize, 1));
            PUT(FTRP(bp), PACK(asize, 1));
            void *next_bp = NEXT_BLKP(bp); 
            PUT(HDRP(next_bp), PACK(csize - asize, 0));
            PUT(FTRP(next_bp), PACK(csize - asize, 0));
            insert_node(next_bp);
            return bp;
        }
    } 
    else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        return bp;
    }
}

void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    size = ((words % 2) ? (words + 1) : words) * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

int get_list_idx(size_t size) {
    size_t exp = 0;
    size_t p = 1;
    if (size < 16) return -1;
    while (p <= size / 2 && exp < 13) {
        p *= 2;
        exp++;
    }
    return exp - 4;
}