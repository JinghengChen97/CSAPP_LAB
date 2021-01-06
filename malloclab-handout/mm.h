#include <stdio.h>
/*
 * Students work in teams of one or two.  Teams enter their team name,
 * personal names and login IDs in a struct of this
 * type in their bits.c file.
 */
typedef struct {
    char *teamname; /* ID1+ID2 or ID1 */
    char *name1;    /* full name of first member */
    char *id1;      /* login ID of first member */
    char *name2;    /* full name of second member (if any) */
    char *id2;      /* login ID of second member */
} team_t;

extern int mm_init (void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);
extern void *mm_realloc(void *ptr, size_t size);
extern void GoThroughList();
extern void* GetHeapListPtr();//for debug

static void *extend_heap(size_t words);
static void *coalesce(void *bp);

static void *find_fit(size_t asize);
static void *place(void* bp, size_t asize);

static int check_block(void* bp);
static void print_block();
static void check_list(void *i, size_t tar);
static void print_list(void *i, long size);

static void InsertNode(void* node);
static void RemoveNode(void* node);

extern void mm_checkheap(int verbose);
static void check_heap(int verbose);

extern team_t team;

