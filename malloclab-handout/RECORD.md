+ 调试：一开始想用GDB，后面觉得太麻烦，于是将整个工程搞成了CMAKE，在clion上调试
+ 注意的点:
	1. c文件用cmake编译是有点讲究的，我就遇到两个问题：(1)怎么指定gcc编译：`set(CMAKE_C_COMPILER gcc)`;(2)怎么链接pthread库：[cmake链接pthread库](https://zhuanlan.zhihu.com/p/128519905)   
	2. [怎么给clion的main传参数](https://blog.csdn.net/ykwjt/article/details/90289753)