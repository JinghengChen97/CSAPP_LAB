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
    "CH team",
    /* First member's full name */
    "CH",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT           8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size)         (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE         (ALIGN(sizeof(size_t)))

#define MAX_HEAP            1024

/* 相关的宏，方便操作 */
#define WSIZE               4
#define DSIZE               8
#define CHUNKSIZE           (1<<12)  //4k

#define MAX(x, y)           ((x) > (y) ? (x) : (y))

#define PACK(size, alloc)   ((size | alloc))
#define GET(p)              (*(unsigned int*)(p))
#define PUT(p, val)         (*(unsigned int*)(p) = (val))

#define GET_SIZE(p)         (GET(p) & ~0x07)  //p指向块的头部或尾部，这会获取p对应块的字节大小
#define GET_ALLOC(p)        (GET(P) & 0X1)    //p指向块的头部或尾部，这会获得p对应块的alloc信息

#define HEADER(bp)          ((char*)(bp) - WSIZE)  //bp指向块的第一个字节,调用这个宏会获得头部
#define FOOTER(bp)          ((char*)(bp) + GET_SIZE(HEADER(bp)) - DSIZE) //bp指向块的第一个字节,调用这个宏会获得尾部

#define NEXT_BLKP(bp)       ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE))) //返回后一个BLOCK的第一个字节（不是头部哦）
#define PREV_BLKP(bp)       ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE))) //返回前一个BLOCK的第一个字节（不是头部哦）

static void* heap_listp;//隐式堆链表的头
/* 
 * mm_init - initialize the malloc package.
 * 1.这一part参考书本的写法
 */
int mm_init(void)
{
    //1.从内存系统中要四个字，这四个字分别是起始块、序言块(两个双字)、终止块
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void*)-1) {
        return -1;
    }

    //2.给这四个块赋相应值
    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
    heap_listp += (2 * WSIZE);

    //3.调extend_heap,尝试扩展堆
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL) return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(ptr);
    PUT(HEADER(ptr), PACK(size, 0));
    PUT(FOOTER(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}


void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1) return NULL;//新申请的内存块是紧跟着原先的终止块的

    PUT(HEADER(bp), PACK(size, 0));//原来的终止块变成了新内存块的头部，因此要将原来的终止块设置一下
    PUT(FOOTER(bp), PACK(size, 0));//新内存块的尾部设置一下
    PUT(HEADER(NEXT_BLKP(bp)), PACK(0, 1));//新的终止块设置一下
    
    /*@todo:内存块合并*/
    return bp;
}

void *coalesce(void *bp) {
    //1.获取前后块的alloc信息
    size_t prev_alloc = GET_ALLOC(HEADER(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(FOOTER(NEXT_BLKP(bp));

    //2.获取当前块的size
    size_t size = GET_SIZE(bp);
    
    //3.根据那四种情况，分类讨论
    ///情况1：上下都被分配,这种情况在调用free时就已经处理了，因此这里不作处理，直接返回
    if (prev_alloc && next_alloc) return;
    
    ///情况2：上被分配，下没分配
    else if (prev_alloc && !next_alloc) {
        //将下面那块跟当前块一起合并
        ///获得下一块与当前块的长度和
        size += GET_SIZE(NEXT_BLKP(bp));

        ///修改当前块的头部
        PUT(HEADER(bp), PACK(size, 0));

        ///修改下一块的尾部
        PUT(FOOTER(NEXT_BLKP(bp)), PACK(size, 0));
        
        return bp;
    }
    ///情况3：上下都没分配
    ///情况4：上没分配，下被分配
}


//思路记录
//1.先了解memlib.c提供的对系统内存管理的模拟模型
//2.编写有关的宏（PUT、PACK等）
//3.写mm_init









