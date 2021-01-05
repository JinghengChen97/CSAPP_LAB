//https://www.jianshu.com/p/48d5d0554b3b
#include "mm.h"
#include "memlib.h"

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


void TesterGoThroughList() {
    void* heap_list = (unsigned int*) GetHeapListPtr();
    unsigned int* tmp_ptr = (unsigned int*) GetHeapListPtr();
    int i = 0;

    tmp_ptr = (unsigned int*)((char*)tmp_ptr - 4 * 2);
    printf("填充字：地址：0x%x, 块大小: %d, 是否被分配：%d\n", tmp_ptr, *tmp_ptr & ~0x07, *tmp_ptr & 0x07);

    tmp_ptr = (unsigned int*)((char*)heap_list - 4 * 1);
    printf("序言块头：地址：0x%x, 块大小: %d, 是否被分配：%d\n", tmp_ptr, *tmp_ptr & ~0x07, *tmp_ptr & 0x07);

    tmp_ptr = (unsigned int*)((char*)heap_list - 4 * 0);
    printf("序言块尾：地址：0x%x, 块大小: %d, 是否被分配：%d\n", tmp_ptr, *tmp_ptr & ~0x07, *tmp_ptr & 0x07);

    tmp_ptr = (unsigned int*)((char*)heap_list);
    while (GET_SIZE(HEADER(tmp_ptr)) != 0) {
        printf("块%d头: 地址：0x%x, 块大小: %d, 是否被分配：%d\n", i, HEADER(tmp_ptr) , GET_SIZE(HEADER(tmp_ptr)), GET_ALLOC(HEADER(tmp_ptr)));
        printf("块%d尾: 地址：0x%x, 块大小: %d, 是否被分配：%d\n", i++, FOOTER(tmp_ptr), GET_SIZE(FOOTER(tmp_ptr)), GET_ALLOC(FOOTER(tmp_ptr)));
        tmp_ptr = NEXT_BLKP(tmp_ptr);
    }

}

int main (int argc, char** argv) {
    mem_init();

    ///1.tests for mm_init： 检查填充字、序言块、结尾块、初始化时多申请的那个块
    printf("///1.tests for mm_init:\n\n\n");
    if (mm_init() == -1) printf("mm_init failed!!\n");
    void* heap_list = GetHeapListPtr();
    /*
        正确答案：
        填充字：地址：0xa1840010, 块大小: 0, 是否被分配：0
        序言块头：地址：0xa1840014, 块大小: 8, 是否被分配：1
        序言块尾：地址：0xa1840018, 块大小: 8, 是否被分配：1
        多申请的块(大小为4096)的头：地址：0xa184001c, 块大小: 4096, 是否被分配：0
        多申请的块(大小为4096)的尾：地址：0xa1841018, 块大小: 4096, 是否被分配：0
        结尾块：地址：0xa184101c, 块大小: 0, 是否被分配：1
    */
    unsigned int* tmp_ptr = (unsigned int*)((char*)heap_list - 4 * 2);
    printf("填充字：地址：0x%x, 块大小: %d, 是否被分配：%d\n", tmp_ptr, *tmp_ptr & ~0x07, *tmp_ptr & 0x07);

    tmp_ptr = (unsigned int*)((char*)heap_list - 4 * 1);
    printf("序言块头：地址：0x%x, 块大小: %d, 是否被分配：%d\n", tmp_ptr, *tmp_ptr & ~0x07, *tmp_ptr & 0x07);

    tmp_ptr = (unsigned int*)((char*)heap_list - 4 * 0);
    printf("序言块尾：地址：0x%x, 块大小: %d, 是否被分配：%d\n", tmp_ptr, *tmp_ptr & ~0x07, *tmp_ptr & 0x07);

    tmp_ptr = (unsigned int*)((char*)heap_list + 4 * 1);
    printf("多申请的块(大小为4096)的头：地址：0x%x, 块大小: %d, 是否被分配：%d\n", tmp_ptr, *tmp_ptr & ~0x07, *tmp_ptr & 0x07);

    tmp_ptr = (unsigned int*)((char*)heap_list + 4096);
    printf("多申请的块(大小为4096)的尾：地址：0x%x, 块大小: %d, 是否被分配：%d\n", tmp_ptr, *tmp_ptr & ~0x07, *tmp_ptr & 0x07);

    tmp_ptr = (unsigned int*)((char*)heap_list + 4096 + 4);
    printf("结尾块：地址：0x%x, 块大小: %d, 是否被分配：%d\n", tmp_ptr, *tmp_ptr & ~0x07, *tmp_ptr & 0x07);

    printf("\n\n\n");

    ///2.test for mm_alloc:
    printf("///2.test for mm_alloc:\n\n\n");

    //2.1 内存块中有足够的空间放下所申请的内存
    printf("//2.1 内存块中有足够的空间放下所申请的内存:\n");
    printf("申请2040个字节x0\n");
    unsigned int* m_alloc_1 = mm_malloc(2040);
    TesterGoThroughList();

    printf("申请2040个字节x1\n");
    unsigned int* m_alloc_2 = mm_malloc(2040);
    TesterGoThroughList();

    printf("再申请48个字节x2\n");
    unsigned int* m_alloc_3 = mm_malloc(48);
    TesterGoThroughList();

    // //2.1 内存块中有足够的空间放下所申请的内存
    // printf("//2.1 内存块中有足够的空间放下所申请的内存:\n");
    // printf("申请1个字节\n");
    // unsigned int* m_alloc_1 = mm_malloc(1);
    // TesterGoThroughList();

    // printf("再申请1个字节\n");
    // unsigned int* m_alloc_2 = mm_malloc(1);
    // TesterGoThroughList();

    // printf("回收最近申请的1个字节\n");
    // mm_free(m_alloc_2);
    // TesterGoThroughList();

    // printf("回收申请的第1个字节\n");
    // mm_free(m_alloc_1);
    // TesterGoThroughList();

    // //2.2 内存块中没有足够的空间放下所申请的内存
    // printf("//2.2 内存块中没有足够的空间放下所申请的内存:\n");

    // printf("申请4096个字节\n");
    // unsigned int* m_alloc_3 = mm_malloc(4096);
    // TesterGoThroughList();

    // printf("释放刚申请的4096个字节\n");
    // mm_free(m_alloc_3);
    // TesterGoThroughList();

    // //2.3 内存块中没有足够的空间放下所申请的内存
    // printf("//2.3 合并：上下都是空闲块的情况\n");

    // printf("申请16个字节x1\n");
    // unsigned int* m_alloc_4 = mm_malloc(16);
    // TesterGoThroughList();

    // printf("申请16个字节x2\n");
    // unsigned int* m_alloc_5 = mm_malloc(16);
    // TesterGoThroughList();

    // printf("申请16个字节x3\n");
    // unsigned int* m_alloc_6 = mm_malloc(16);
    // TesterGoThroughList();

    // printf("释放刚申请的16个字节x2\n");
    // mm_free(m_alloc_5);
    // TesterGoThroughList();

    // printf("释放刚申请的16个字节x3\n");
    // mm_free(m_alloc_6);
    // TesterGoThroughList();

////    //2.1 内存块中有足够的空间放下所申请的内存
//    printf("//2.1 内存块中有足够的空间放下所申请的内存:\n");
//    printf("申请2040个字节x0\n");
//    unsigned int* m_alloc_0 = mm_malloc(2040);
//    TesterGoThroughList();
//
//    printf("申请2040个字节x1\n");
//    unsigned int* m_alloc_1 = mm_malloc(2040);
//    TesterGoThroughList();
//
//    printf("释放2040个字节x1\n");
//    mm_free(m_alloc_1);
//    TesterGoThroughList();
//
//    printf("申请48个字节x2\n");
//    unsigned int* m_alloc_2 = mm_malloc(48);
//    TesterGoThroughList();
//
//    printf("申请4072个字节x3\n");
//    unsigned int* m_alloc_3 = mm_malloc(4072);
//    TesterGoThroughList();
//
//    printf("释放4072个字节x3\n");
//    mm_free(m_alloc_3);
//    TesterGoThroughList();
//
//    printf("申请4072个字节x4\n");
//    unsigned int* m_alloc_4 = mm_malloc(4072);
//    TesterGoThroughList();
//
//    printf("释放2040个字节x0\n");
//    mm_free(m_alloc_0);
//    TesterGoThroughList();
//
//    printf("释放48个字节x2\n");
//    mm_free(m_alloc_2);
//    TesterGoThroughList();
//
//    printf("申请4072个字节x5\n");
//    unsigned int* m_alloc_5 = mm_malloc(4072);
//    TesterGoThroughList();
//
//    printf("释放4072个字节x4\n");
//    mm_free(m_alloc_4);
//    TesterGoThroughList();
//
//    printf("释放4072个字节x5\n");
//    mm_free(m_alloc_5);
//    TesterGoThroughList();
}