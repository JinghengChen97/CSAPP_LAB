#include "cachelab.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h> 
#include <limits.h>

#define VALID_FLAG 0
#define TAG_FLAG   1
#define LRU_VALUE  2

int e = 0, s = 0, b = 0;

//是否打印Traces的信息；
int print_traces = 0;

// cache 的数据结构及相关操作
struct _Cache{
    int S;//cache的组数
    int E;//cache的行数
    int B;//cache每一块的大小
    int*** cache;//cache[s][e][0]:有效位，cache[s][e][1]:标记位，cache[s][e][2]:LRU计数
} sim_cache;


struct _LabResult{
    unsigned int num_hit;
    unsigned int num_miss;
    unsigned int num_eviction;
} lab_result;

void cache_init() {
    sim_cache.S = (1 << s);
    sim_cache.E = e;
    sim_cache.B = b;
    sim_cache.cache = (int***) malloc(sizeof(int**) * sim_cache.S);
	for(int i = 0; i < sim_cache.S; i++) sim_cache.cache[i] = (int**)malloc(sizeof(int*) * sim_cache.E);
    for (int i = 0; i < sim_cache.S; i++) {
        for (int j = 0; j < sim_cache.E; j++) {
            sim_cache.cache[i][j] = (int*)malloc(sizeof(int) * 3);
        }
    }
}

void UpdateCache(unsigned int addr) {
    //L、S：访问一次cache；M：访问两次cache
    unsigned int s_index = (addr >> sim_cache.B) & ((-1U) >> (32 - s));
    unsigned int t_index = (addr >> (s + sim_cache.B));

    //先看看当前组的cache里有没有目标addr的副本
    for (int i = 0; i < sim_cache.E; i++) {
        if ((sim_cache.cache[s_index][i][VALID_FLAG] == 1) && (sim_cache.cache[s_index][i][TAG_FLAG] == t_index)) {
            lab_result.num_hit++;
            sim_cache.cache[s_index][i][LRU_VALUE] = 0;

            printf("Hit ");
            return;
        }
    }
    //如果没有，看看有没有空的行可以放进去
    for (int i = 0; i < sim_cache.E; i++) {
        if (sim_cache.cache[s_index][i][VALID_FLAG] == 0) {
            lab_result.num_miss++;
            sim_cache.cache[s_index][i][TAG_FLAG] = t_index;
            sim_cache.cache[s_index][i][VALID_FLAG] = 1;
            sim_cache.cache[s_index][i][LRU_VALUE] = 0;

            printf("Miss ");
            return;
        }
    }
    //如果没有空的，驱逐LRU值最大的行
    int max_LRU = INT_MIN;
    int max_LRU_index = -1;
    for (int i = 0; i < sim_cache.E; i++) {
        if (sim_cache.cache[s_index][i][LRU_VALUE] > max_LRU) {
            max_LRU = sim_cache.cache[s_index][i][LRU_VALUE];
            max_LRU_index = i;
        }
    }
    sim_cache.cache[s_index][max_LRU_index][LRU_VALUE] = 0;
    sim_cache.cache[s_index][max_LRU_index][TAG_FLAG] = t_index;
    lab_result.num_eviction++;
    lab_result.num_miss++;
    printf("Miss Eviction");
}


// traces文件的读取、对应操作

void VerboseTracesInfo() {
    print_traces = 1;
}

void print_traces_info(char* buffer) {
    printf("%s\n", buffer);
}

void UpdateLRU() {
    for(int i = 0; i < sim_cache.S; i++)
        for(int j = 0; j < sim_cache.E; j++)
            if(sim_cache.cache[i][j][VALID_FLAG] == 1)//if valid
                sim_cache.cache[i][j][LRU_VALUE]++;
}

void PrintUsage() {
    /*
        Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>
        Options:
        -h         Print this help message.
        -v         Optional verbose flag.
        -s <num>   Number of set index bits.
        -E <num>   Number of lines per set.
        -b <num>   Number of block offset bits.
        -t <file>  Trace file.

        Examples:
        linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace
        linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace
    */
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional verbose flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n\n");
    printf("Examples:\n");
    printf("linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

int main(int argc, char** argv)
{
    //读取命令行设置(用法是有框架的,参考链接https://www.cnblogs.com/qingergege/p/5914218.html)
    //包括：
    ///1.设置cache的参数
    ///2.获取help,verbose的设置
    ///3.获取traces文档路径
    int opt;
    char trace_file_path[1000];
    while (-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))) {
        switch(opt) {
            case 'h': {
                PrintUsage();
                return 0;
            }
            case 'v': {
                VerboseTracesInfo();//打印Trace的信息
                break;
            }
            case 's': {
                s = atoi(optarg);
                break;
            }
            case 'E': {
                e = atoi(optarg);
                break;
            }
            case 'b': {
                b = atoi(optarg);
                break;
            }
            case 't': {
                strcpy(trace_file_path, optarg);//必须要复制
                break;
            }
            default: {
                printf("Incorrect input!!\n\n");
                PrintUsage();
                return 0;
            }
        }
    }
    
    //根据读取的参数，初始化cache
    //包括：
    ///1.给cache分配内存
    cache_init();

    //读取文本，进行对应操作
    //包括：
    ///1.初始化lab_result
    ///2.打开文件
    ///3.读取每一行的内容，根据每一行内容执行不同操作
    lab_result.num_eviction = 0;
    lab_result.num_hit = 0;
    lab_result.num_miss = 0;

    FILE* fp = fopen(trace_file_path,"r");
	if(fp == NULL)
	{
		fprintf(stderr,"Fail to open the file!!\n");
		exit(-1);
	}
    char buffer[1000];
    char operation_type;
    unsigned int addr, size;
    while (fgets(buffer, 1000, fp)) {
        sscanf(buffer, " %c %xu,%d", &operation_type, &addr, &size);
        switch(operation_type) {
            case 'I': {
                break;
            }
            case 'M': {
                UpdateCache(addr);
                UpdateCache(addr);
                break;
            }
            case 'L': {
                UpdateCache(addr);
                break;
            }
            case 'S': {
                UpdateCache(addr);
                break;
            }
            default: {
                break;
            }
        }
        UpdateLRU();
        print_traces_info(buffer);
    }


    //打印最后结果
    printSummary(lab_result.num_hit, lab_result.num_miss, lab_result.num_eviction);
    return 0;
}

