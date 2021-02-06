#ifndef __CACHE_H__
#define __CACHE_H__

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define LRU_MAGIC_NUMBER 9999
#define CACHE_OBJS_COUNT 10

#include "csapp.h"

/* $begin cache_block_t */
typedef struct {
    int read_cnt;               //当前有多少读者
    int LRU;                    //LRU值，每次访问缓存时，没有被击中的cache_block都会自增1
    int is_empty;               //此cache是否为空
    sem_t mutex;                //read_cnt的锁
    sem_t w;                    //写LRU cache的锁
    char uri[MAXLINE];          //cache block对应的uri
    char obj[MAX_OBJECT_SIZE];  //缓存内容
} cache_block_t;
/* $end cache_block_t */

/* $begin cache_t */
typedef struct {
    cache_block_t cache_objects[CACHE_OBJS_COUNT];
} cache_t;
/* $end cache_t */

void cache_init(cache_t *cache);
int  cache_find(cache_t *cache, char *uri);                 //查找有无cache被击中
int  cache_eviction(cache_t *cache);                        //找空的或最大LRU的cache_block,返回值是这个block的索引
void cache_store(cache_t *cache, char *uri, char *buf);     //将buf和uri的东西copy到一个可用cache中去 
void cache_update_lru(cache_t *cache, int index);               //更新除了i号cache以外的所有cache的LRU
void read_pre(cache_t *cache, int i);
void read_after(cache_t *cache, int i);
void write_pre(cache_t *cache, int i);
void write_after(cache_t *cache, int i);
#endif  /* __CACHE_H__*/
