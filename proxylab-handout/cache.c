#include "csapp.h"
#include "cache.h"
#define IS_EMPTY      1
#define IS_NOT_EMPTY  0

void cache_init(cache_t *cache) {
    int i;
    for (i = 0; i < CACHE_OBJS_COUNT; i++) {
        cache->cache_objects[i].is_empty = IS_EMPTY;
        cache->cache_objects[i].LRU = 0;
        cache->cache_objects[i].read_cnt = 0;
        sem_init(&(cache->cache_objects[i].mutex), 0, 1);
        sem_init(&(cache->cache_objects[i].w), 0, 1);
    }
}

void read_pre(cache_t *cache, int i) {
    //锁上read_cnt
    P(&cache->cache_objects[i].mutex);

    //如果是第一个读者，锁上w（有人在读就不给写）
    if ((cache->cache_objects[i].read_cnt++) == 1) {
        P(&cache->cache_objects[i].w);
    }
    
    //解锁read_cnt
    V(&cache->cache_objects[i].mutex);
}

void read_after(cache_t *cache, int i) {
    //锁上read_cnt
    P(&cache->cache_objects[i].mutex);

    //如果是最后一个读者，解锁W（没人在读就给写）
    if ((cache->cache_objects[i].read_cnt--) == 0) {
        V(&cache->cache_objects[i].w);
    }
    
    //解锁read_cnt
    V(&cache->cache_objects[i].mutex);
}

void write_pre(cache_t *cache, int i) {
    P(&cache->cache_objects[i].w);
}

void write_after(cache_t *cache, int i) {
    V(&cache->cache_objects[i].w);
}

int cache_find(cache_t* cache, char* uri) {
    int i, goal_index = -1;
    for (i = 0; i < CACHE_OBJS_COUNT; i++) {
        //查找某个cache是一个读的过程，因此要先上锁
        read_pre(cache, i);

        //根据uri是否相同且该block是否有效来判断击中与否
        //如果击中，记录这个cache的索引
        if ((cache->cache_objects[i].is_empty == IS_NOT_EMPTY) && 
            (strcmp(uri, cache->cache_objects[i].uri) == 0)) {
                goal_index = i;
                break;//这时这个cache仍要保持上锁状态
            }
        //解锁
        read_after(cache, i);
    }
    //如果没找到，返回-1
    return goal_index;
}

int cache_eviction(cache_t *cache) {
    int i, max_LRU_index = -1, max_LRU = -1;

    //找空的或最大LRU的cache，返回其索引
    for (i = 0; i < CACHE_OBJS_COUNT; i++) {
        read_pre(cache, i);
        if (cache->cache_objects[i].is_empty == IS_EMPTY) {
            max_LRU_index = i;
            read_after(cache, i);
            break;
        }
        if (cache->cache_objects[i].LRU > max_LRU) {
            //找到了最大LRU的block之后要给他上一个读锁，等到他被换下才解锁
            //防止在函数返回之后，此block被写入之前被其他写者改了
            if (max_LRU_index != -1) read_after(cache, max_LRU_index);
            max_LRU = cache->cache_objects[i].LRU;
            max_LRU_index = i;
        }
    }
    return max_LRU_index;
}

void cache_store(cache_t *cache, char *uri, char *buf) {
    int goal_cache_index = cache_eviction(cache);
    read_after(cache, goal_cache_index);
    
    write_pre(cache, goal_cache_index);

    strcpy(cache->cache_objects[goal_cache_index].uri, uri);
    strcpy(cache->cache_objects[goal_cache_index].obj, buf);
    cache->cache_objects[goal_cache_index].is_empty = IS_NOT_EMPTY;
    cache->cache_objects[goal_cache_index].LRU = 0;

    cache_update_lru(cache, goal_cache_index);

    write_after(cache, goal_cache_index);
}

void cache_update_lru(cache_t *cache, int index) {
    for (int i = 0; i < index; i++) {
        write_pre(cache, i);
        if (i != index && cache->cache_objects[i].is_empty != IS_EMPTY) {
            cache->cache_objects[i].LRU++;
        }
        write_after(cache, i);
    }
}