### 把工程放到clion中去
+ 调试：一开始想用GDB，后面觉得太麻烦，于是将整个工程搞成了CMAKE，在clion上调试
+ 注意的点:
	1. c文件用cmake编译是有点讲究的，我就遇到两个问题：(1)怎么指定gcc编译：`set(CMAKE_C_COMPILER gcc)`;(2)怎么链接pthread库：[cmake链接pthread库](https://zhuanlan.zhihu.com/p/128519905)   
	2. [怎么给clion的main传参数](https://blog.csdn.net/ykwjt/article/details/90289753)


### 最后还是决定用gdb来debug

#### debug阶段1：
+ 报错：
~~~
Program received signal SIGSEGV, Segmentation fault.
0x08049f93 in coalesce (bp=0xf69f0030) at mm.c:195
195	        PUT(HEADER(PREV_BLKP(bp)), PACK(size, 0));
(gdb) backtrace
#0  0x08049f93 in coalesce (bp=0xf69f0030) at mm.c:195
#1  mm_free (ptr=0xf69f0030) at mm.c:125
#2  0x08048ef0 in eval_mm_valid (ranges=0xffffc9e0, tracenum=<optimised out>, 
    trace=<optimised out>) at mdriver.c:674
#3  main (argc=2, argv=0xffffcab4) at mdriver.c:296

~~~
+ 195行是对前一个块的头写入合并后的size，既然这里发生段错误，那么说明`HEADER(PREV_BLKP(bp))`不是一个正确的地址。于是看看这里会给出什么东西：
+ 先看`bp`和`PREV_BLKP(bp)`:
~~~
(gdb) p /x *(int)(bp)  //bp
$25 = 0xd0d0d0d0
(gdb) p /x *(int)(bp - 1)  //HEADER(bp)
$26 = 0xd0d0d000
(gdb) p /x *(int)(bp - 2)  //FOOTER(PREV_BLKP(bp))
$27 = 0xd0d00000
(gdb) 
~~~
+ 0xd0d0是一个特定的值，代表的是无用数据[0xd0d0d0d0的含义](https://us-cert.cisa.gov/bsi/articles/knowledge/coding-practices/phkmalloc),因此我怀疑bp出了问题
+ 那么怎么调试呢？第一个想法，看看为什么会传进一个有问题的bp形参，于是尝试看看`mm_free`发生了什么事，于是打开`mm_free`的栈帧：[如何用GDB查看栈帧](https://blog.csdn.net/small_prince_/article/details/80682110):
~~~
(gdb) backtrace
#0  0x0804ade6 in coalesce (bp=0xf69f0030) at mm.c:195
#1  0x0804ab64 in mm_free (ptr=0xf69f0030) at mm.c:125
#2  0x08049ec8 in eval_mm_valid (trace=0x8050548, tracenum=0, 
    ranges=0xffffc974) at mdriver.c:674
#3  0x08049097 in main (argc=2, argv=0xffffcab4) at mdriver.c:296
(gdb) frame 1
#1  0x0804ab64 in mm_free (ptr=0xf69f0030) at mm.c:125
125	_T_SIZE);
(gdb) ptr
Undefined command: "ptr".  Try "help".
(gdb) p /x ptr
$15 = 0xf69f0030
~~~
+ `mm_free`输入的ptr看上去没啥问题，看看`mm_free`的局部参数是什么情况先
~~~
(gdb) info locals
size = 0
~~~
+ ？？？`size`为什么是0?那只能说明是`ptr`的`header`出问题咯，马上看看：
~~~
(gdb) p /x *(0xf69f0030 - 1)
$17 = 0xd0d0d000
~~~
`HEADER(ptr)`出错，怀疑是这一块内存在分配时没有组织好头部尾部，于是检查`mm_alloc`:
~~~c++
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
#ifdef DEBUG
    printf("mm_malloc success!!\n\n");
#endif
}
~~~
可见,当前的`mm_alloc`在分配内存时是不会设置头部尾部的，破案。
接下来修改`mm_alloc`。