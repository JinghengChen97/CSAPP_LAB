#include "mm.h"
#include "memlib.h"

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

    tmp_ptr = (unsigned int*)((char*)heap_list + 4 * 1);
    while ((*tmp_ptr & ~0x07) != 0) {
        printf("块%d: 地址：0x%x, 块大小: %d, 是否被分配：%d\n", i++, tmp_ptr, *tmp_ptr & ~0x07, *tmp_ptr & 0x07);
        tmp_ptr = (unsigned int*)((char*)tmp_ptr + (*tmp_ptr & ~0x07));
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
    unsigned int* m_alloc_1 = mm_malloc(1);
    TesterGoThroughList();
    unsigned int* m_alloc_2 = mm_malloc(1);
    TesterGoThroughList();
}