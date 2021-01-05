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
#define WSIZE               4 //单字，4字节（一个字等于多少个字节，与系统硬件（总线、cpu命令字位数等）有关，不应该毫无前提地说一个字等于多少位。参考链接https://blog.csdn.net/fabulous1111/article/details/79525384）
#define DSIZE               8 //双字
#define CHUNKSIZE           (1<<12)  //4k

#define MAX(x, y)           ((x) > (y) ? (x) : (y))

#define PACK(size, alloc)   ((size | alloc))
#define GET(p)              (*(unsigned int*)(p))
#define PUT(p, val)         (*(unsigned int*)(p) = (val))

#define GET_SIZE(p)         (GET(p) & ~0x07)  //p指向块的头部或尾部，这会获取p对应块的字节大小
#define GET_ALLOC(p)        (GET(p) & 0X1)    //p指向块的头部或尾部，这会获得p对应块的alloc信息

#define HEADER(bp)          ((char*)(bp) - WSIZE)  //bp指向块的第一个字节,调用这个宏会获得头部
#define FOOTER(bp)          ((char*)(bp) + GET_SIZE(HEADER(bp)) - DSIZE) //bp指向块的第一个字节,调用这个宏会获得尾部

#define NEXT_BLKP(bp)       ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE))) //返回后一个BLOCK的第一个字节（不是头部哦）
#define PREV_BLKP(bp)       ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE))) //返回前一个BLOCK的第一个字节（不是头部哦）

//#define DEBUG
#ifdef  DEBUG
#define DBG_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#define CHECK_HEAP(verbose) mm_checkheap(verbose)
#else
#define DBG_PRINTF(...)
#define CHECK_HEAP(verbose)
#endif

static void* heap_listp;//隐式堆链表的头
static void* previous_listp;//上一次成功匹配的块指针
/* 
 * mm_init - initialize the malloc package.
 * 1.这一part参考书本的写法
 */
int mm_init(void)
{
    DBG_PRINTF("mm_init\n");
    //1.从内存系统中要四个字，这四个字分别是起始块、序言块(两个双字)、终止块
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void*)-1) {
        return -1;
    }

    //2.给这四个块赋相应值
    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
    heap_listp += (2 * WSIZE);//指向序言块

    //3.调extend_heap,尝试扩展堆
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL) return -1;

    previous_listp = heap_listp;
    //check heap
    CHECK_HEAP(1);
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

    DBG_PRINTF("mm_alloc\n");
    /* 处理无效请求 */
    if (size == 0) {
        return NULL;
    }
    
    /* 调整块大小，调整策略：如果请求大小小于最小块，那么直接改为最小块的size */
    if (size <= DSIZE) asize = 2 * DSIZE;
    else asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    /* 在隐式链表中寻找合适大小的块 */
    if (((bp = find_fit(asize)) != NULL)) {
        place(bp, asize);
        return bp;
    }

    /* 如果找不到合适的块，则申请更多的内存 */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL) return NULL;
    place(bp, asize);

    //check heap
    CHECK_HEAP(1);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HEADER(ptr));

    DBG_PRINTF("mm_free\n");
    PUT(HEADER(ptr), PACK(size, 0));
    PUT(FOOTER(ptr), PACK(size, 0));
    coalesce(ptr);
    
    //check heap
    CHECK_HEAP(1);
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
//    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    copySize = GET_SIZE(HEADER(ptr));
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

    //check heap
    CHECK_HEAP(1);

    return bp;
}

void *coalesce(void *bp) {
    //1.获取前后块的alloc信息
    size_t prev_alloc = GET_ALLOC(FOOTER(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HEADER(NEXT_BLKP(bp)));

    //2.获取当前块的size
    size_t size = GET_SIZE(HEADER(bp));
    
    //3.根据那四种情况，分类讨论
    ///情况1：上下都被分配,这种情况在调用free时就已经处理了，因此这里不作处理，直接返回
    if (prev_alloc && next_alloc) return bp;
    
    ///情况2：上被分配，下没分配
    else if (prev_alloc && !next_alloc) {
        //将下面那块跟当前块一起合并
        ///获得下一块与当前块的长度和
        size += GET_SIZE(HEADER(NEXT_BLKP(bp)));

        ///修改当前块的头部
        PUT(HEADER(bp), PACK(size, 0));

        ///修改下一块的尾部
        PUT(FOOTER(bp), PACK(size, 0));//这里用FOOT(bp)可以直接得到下一块的尾部，因为FOOTER是根据HEADER来跳转的，而HEADER在上面已经被修改了
    }
    ///情况3：上下都没分配
    else if (!prev_alloc && !next_alloc) {
        size += GET_SIZE(FOOTER(NEXT_BLKP(bp))) + GET_SIZE(HEADER(PREV_BLKP(bp)));
        PUT(FOOTER(NEXT_BLKP(bp)), PACK(size, 0));
        PUT(HEADER(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    ///情况4：上没分配，下被分配
    else {
        size += GET_SIZE(HEADER(PREV_BLKP(bp)));
        PUT(HEADER(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FOOTER(bp), PACK(size, 0));//这里用FOOT(bp)可以直接得到下一块的尾部，因为FOOTER是根据HEADER来跳转的，而HEADER在上面已经被修改了
        bp = PREV_BLKP(bp);
    }
    //check heap
    CHECK_HEAP(1);
    return bp;
}

/* 放置请求块到空闲块上
   过程：（1）如果请求块与空闲块之间的空间差大于或等于一个最小块，则将这个最小块分割出去
         （2）如果空闲块刚好能放下请求块，那么直接放下即可
 */
void place(void* bp, size_t asize) {
    int csize = GET_SIZE(HEADER(bp));
    if ((csize - asize) >= (2 * DSIZE)) {
        PUT(HEADER(bp), PACK(asize, 1));
        PUT(FOOTER(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HEADER(bp), PACK(csize - asize, 0));
        PUT(FOOTER(bp), PACK(csize - asize, 0));
    } else {
        PUT(HEADER(bp), PACK(csize, 1));
        PUT(FOOTER(bp), PACK(csize, 1));
    }
}
//p *(unsigned int*)((char*) bp - 4)
/*
  寻找合适大小的空闲块
  策略：下次适配
*/
void* find_fit(size_t asize) {
    // void* bp;
    // for (bp = heap_listp; GET_SIZE(HEADER(bp)) > 0; bp = NEXT_BLKP(bp)) {
    //     if ((!GET_ALLOC(HEADER(bp))) && (GET_SIZE(HEADER(bp)) >= asize)) {
    //        return bp;
    //     }
    // }

    void* bp = previous_listp;
    for (; GET_SIZE(HEADER(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if ((!GET_ALLOC(HEADER(bp))) && (GET_SIZE(HEADER(bp)) >= asize)) {
           return bp;
        }
    }
    for (bp = heap_listp; GET_SIZE(HEADER(bp)) > 0 && bp != previous_listp; bp = NEXT_BLKP(bp)) {
        if ((!GET_ALLOC(HEADER(bp))) && (GET_SIZE(HEADER(bp)) >= asize)) {
           return bp;
        }
    }
    return NULL;
}

void GoThroughList() {
    void* bp = heap_listp;
    int i = 0;
    for (i = 0; GET_SIZE(HEADER(bp)) > 0; bp = NEXT_BLKP(bp), i++) {
        printf("Current index is %d\n", i);
        printf("Current block is %X\n", bp);
        printf("Current header is %X\n", HEADER(bp));
        printf("Current footer is %X\n", FOOTER(bp));
        if (*(unsigned int*)(HEADER(bp)) == *(unsigned int*)(FOOTER(bp))) {
            printf("Block %d's Footer and Header is the same.\n", i);
        } else {
            printf("Block %d's Footer and Header is NOT the same!!!\n", i);
        }
        printf("\n\n\n");
    }
}

void* GetHeapListPtr() {
    return heap_listp;
}

int check_block(void *bp)
{
    //area is aligned
    if ((size_t)bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    //header and footer match
    if (GET(HEADER(bp)) != GET(FOOTER(bp)))
        printf("Error: header does not match footer\n");
    size_t size = GET_SIZE(HEADER(bp));
    //size is valid
    if (size % 8)
        printf("Error: %p payload size is not doubleword aligned\n", bp);
    return GET_ALLOC(HEADER(bp));
}

void print_block(void *bp)
{
    long int hsize, halloc, fsize, falloc;

    hsize = GET_SIZE(HEADER(bp));
    halloc = GET_ALLOC(HEADER(bp));
    fsize = GET_SIZE(FOOTER(bp));
    falloc = GET_ALLOC(FOOTER(bp));

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%ld:%c] footer: [%ld:%c]\n", bp,
           hsize, (halloc ? 'a' : 'f'),
           fsize, (falloc ? 'a' : 'f'));
}

void mm_checkheap(int verbose)
{
    check_heap(verbose);
}

void check_heap(int verbose) {
    char *bp = heap_listp;

    if (verbose)
        printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HEADER(heap_listp)) != DSIZE) || !GET_ALLOC(HEADER(heap_listp)))
        printf("Bad prologue header\n");
    // block level
    check_block(heap_listp);
    int pre_free = 0;
    for (bp = heap_listp; GET_SIZE(HEADER(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose)
            print_block(bp);
        int cur_free = check_block(bp);
        //no contiguous free blocks
        if (pre_free && cur_free) {
            printf("Contiguous free blocks\n");
        }
    }
    if (verbose)
        print_block(bp);
    if ((GET_SIZE(HEADER(bp)) != 0) || !(GET_ALLOC(HEADER(bp))))
        printf("Bad epilogue header\n");
}