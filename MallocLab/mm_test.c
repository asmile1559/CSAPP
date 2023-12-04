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
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4 bytes) or double word (8 bytes) alignment */
#define ALIGNMENT 8 // all malloc space must align to dword

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* for header and footer */
#define PICK(size, alloc) ((size) | (alloc))

/* word size = 4 bytes, double word size = 8 bytes*/
#define WSIZE 4             // word is 4 bytes -> 1 int
#define DSIZE 8             // dword is 8 bytes -> 2 int
#define CHUNKSIZE (1 << 12) // 4kByte

/* read and write data */
#define GET(p) (*(unsigned int *)(p))
#define SET(p, val) ((*(unsigned int *)(p)) = (val))

/* read and set header/footer */
#define GET_SIZE(p) ((*(unsigned int *)(p)) & ~0x7) // header and footer is 4byte space
#define GET_ALLOC(p) ((*(unsigned int *)(p)) & 0x1)

/* get header or footer address from block address(bp) */
#define HDRP(bp) ((char *)(bp)-WSIZE) // use (char *) means (bp - 4 bytes)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* next block header/block pointer, prev header/block pointer used in expend_heap*/
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define NEXT_BLKH(bp) (HDRP(NEXT_BLKP(bp)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-DSIZE))
#define PREV_BLKH(bp) (HDRP(PREV_BLKP(bp)))

/* data size and block size */
#define DATASIZE(bs) ((bs)-DSIZE)
#define BLOCKSIZE(ds) ((ds) + DSIZE)

/* pred and succ pointer */
#define PRED_PTR(bp) ((unsigned long *)(bp))
#define SUCC_PTR(bp) ((unsigned long *)((char *)(bp) + DSIZE))
#define GET_PRED_PTR(bp) (*PRED_PTR(bp))
#define SET_PRED_PTR(bp, val) (GET_PRED_PTR(bp) = (val))
#define GET_SUCC_PTR(bp) (*SUCC_PTR(bp))
#define SET_SUCC_PTR(bp, val) (GET_SUCC_PTR(bp) = (val))

/* get address table from bias */
#define SUP(n) ((1 << (n + 3))) // max payload(bytes) in n-th table [8, 16, ... 2048, 4096], max{n} = 9
#define SETDWORD(p, val) ((*(unsigned long *)(p)) = (val))
#define GETDWORD(p) ((*(unsigned long *)(p)))

/* address tables */
#define TABLE_CNT 10
#define ADDR_TABLE_N(at, n) ((char *)(at) + (n) * DSIZE)
// #define AT8(at) ADDR_TABLE_N(at, 0)
#define AT16(at) ADDR_TABLE_N(at, 0)   // 8~15
#define AT32(at) ADDR_TABLE_N(at, 1)   // 16~31
#define AT64(at) ADDR_TABLE_N(at, 2)   // 32~63
#define AT128(at) ADDR_TABLE_N(at, 3)  // 64~127
#define AT256(at) ADDR_TABLE_N(at, 4)  // 127~255
#define AT512(at) ADDR_TABLE_N(at, 5)  // 255~511
#define AT1024(at) ADDR_TABLE_N(at, 6) // 511~1023
#define AT2048(at) ADDR_TABLE_N(at, 7) // 1024~2047
#define AT4096(at) ADDR_TABLE_N(at, 8) // 2047~4095
#define ATM(at) ADDR_TABLE_N(at, 9)    // 4096~inf

#define EOFAT(at) ADDR_TABLE_N(at, TABLE_CNT)
#define NEXTAT(tp) (((char *)(tp) + DSIZE))

/* functions */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);

/* helper */
static void search_and_remove(char *atable, void *bp);
static void *get_appropriate_table(char *atable, size_t data_size);
static void *split_block(void *p, size_t bsize);

/*
memlib.c

void *mem_sbrk(int incr): incr是一个非零正整数, 返回指向第一个内存空间的地址.这个函数模仿sbrk函数,只不过incr只能为非零正整数
void *mem_heap_lo(void):返回heap的首地址
void *mem_heap_high(void):返回heap的尾地址
size_t mem_heapsize(void):返回heap的大小
size_t mem_pagesize(void):返回系统页的大小(4K byte 在 linux)

*/

char *heap_list; // 指向堆头数据位置
char *atable;    // 地址表的起始位置

/*
 * mm_init - initialize the malloc package.
 * mm_init的任务
 * 1. 分配一个初始的block, 包含prologue, 地址表和epilogue
 * 2. 通过expend_heap,初始化一个大小为4kB的空白块
 */

int mm_init(void)
{
    heap_list = mem_sbrk(TABLE_CNT * DSIZE + 2 * DSIZE); // 分配12个双字(1个地址是64位,占用一个双字的大小)

    char *ptr = heap_list;              // 活动指针
    atable = heap_list + WSIZE + DSIZE; // 地址表起始位置
    // zero fill
    SET(ptr, 0);

    // prologue
    SET(ptr + (1 * WSIZE), PICK(DSIZE, 1));
    SET(ptr + (2 * WSIZE), PICK(DSIZE, 1));

    // explicit address table
    for (int i = 0; i < TABLE_CNT; i++)
    {
        SETDWORD(atable + i * DSIZE, NULL); // 初始化地址表地址, 设置指向地址为NULL
    }

    // epilogue
    SET(ptr + (WSIZE + DSIZE + TABLE_CNT * DSIZE), PICK(0, 1));

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    {
        return -1;
    }

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t dsize = ALIGN(size); // data size (bytes)
    size_t bsize = BLOCKSIZE(dsize);
    void *tp = get_appropriate_table(atable, dsize);
    void *p;
    void *bp = NULL;
    void *eofat = EOFAT(atable);
    while (tp != eofat)
    {
        p = GETDWORD(tp);
        if (p != NULL)
        {
            // first fit
            while (p != NULL)
            {
                if ((bp = split_block(p, bsize)) != NULL)
                {
                    return bp;
                }
                p = SUCC_PTR(p);
            }
        }
        tp = NEXTAT(tp);
    }

    // 如果没有合适的块, 或空间不足
    if ((bp = extend_heap(CHUNKSIZE / WSIZE)) == NULL)
    {
        return bp;
    }

    // 新的空间申请成功后就有合适的block进行分配了
    p = GETDWORD(ATM(atable));

    // 成功返回 bp， 失败返回NULL
    bp = split_block(p, bsize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t bsize = GET_SIZE(HDRP(ptr));
    SET(HDRP(ptr), PICK(bsize, 0));
    SET(FTRP(ptr), PICK(bsize, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (size == 0)
    {
        mm_free(ptr);
        return NULL;
    }

    if (ptr == NULL)
    {
        return mm_malloc(size);
    }

    void *newptr = NULL;
    void *oldptr = ptr;
    size_t old_bsize = GET_SIZE(HDRP(oldptr));
    size_t new_bsize = BLOCKSIZE(ALIGN(size));

    if(old_bsize == new_bsize)
        return oldptr;

    if(!GET_ALLOC(NEXT_BLKH(oldptr)))
    {
        // 如果后面一块没有被分配
        if(new_bsize < old_bsize)
        {
            search_and_remove(atable, NEXT_BLKP(oldptr));

            // 只要新块小于旧块,就进行分割合并
            SET(HDRP(oldptr), PICK(new_bsize, 1));
            SET(FTRP(oldptr), PICK(new_bsize, 1));

            newptr = oldptr;
            oldptr = NEXT_BLKP(newptr);

            SET(HDRP(oldptr), PICK(old_bsize - new_bsize, 0));
            SET(FTRP(oldptr), PICK(old_bsize - new_bsize, 0));
        }
        else
        {
            size_t remain_size = new_bsize - old_bsize;
            size_t total_size = GET_SIZE(NEXT_BLKH(oldptr)) + GET_SIZE(HDRP(oldptr));

            if (total_size >= new_bsize + 2 * DSIZE)
            {
                // remain_size + (4 + 8 + 8) - 8 + 4 -> remain_size + 12
                // 如果新块大于旧块, 且旧块+free block的大小足够容纳新块,那么就进行分割
                search_and_remove(atable, NEXT_BLKP(oldptr));
                
                SET(HDRP(oldptr), PICK(new_bsize, 1));
                SET(FTRP(oldptr), PICK(new_bsize, 1));

                newptr = oldptr;
                oldptr = NEXT_BLKP(newptr);

                SET(HDRP(oldptr), PICK(total_size - new_bsize, 0));
                SET(FTRP(oldptr), PICK(total_size - new_bsize, 0));
            }
            else
            {
                newptr = mm_malloc(size);
                if (newptr == NULL)
                {
                    return NULL;
                }
                memcpy(newptr, oldptr, DATASIZE(old_bsize));
                mm_free(oldptr);
            }
        
        }
        coalesce(oldptr);
    }
    else
    {
        if (new_bsize < old_bsize && old_bsize < new_bsize + 16)
        {
            // 如果新块的大小小于旧块,且旧块比新块大不超过16bytes
            // 方法1: 重新分配一个大小等于new_bsize的block进行拷贝
            // 但是我认为没有必要这样做,除非为了提高利用率;
            // 方法2: 直接使用旧块,由于后面一块被分配了,新隔离出的部分
            // 也没有地方存放,因此选择直接返回旧块指针;
            newptr = oldptr;
        }
        else if (new_bsize < old_bsize && old_bsize >= new_bsize + 16)
        {
            newptr = split_block(oldptr, new_bsize);
        }
        else
        {
            newptr = mm_malloc(size);
            if (newptr == NULL)
            {
                return NULL;
            }
            memcpy(newptr, oldptr, DATASIZE(old_bsize));
            mm_free(oldptr);
        }
    }     
    return newptr;
}

/**
 * 拓展堆的大小,当mm_malloc分配的空间超过当前的最大空间时,需要拓展堆的可用空间
 */
static void *extend_heap(size_t words)
{
    char *bp;    // 新的chunk的首指针
    size_t size; // 规范化后的字节大小

    // 如果拓展字节是奇数, 将其补全为偶数
    size = (words & 0x1) ? (words + 1) * WSIZE : words * WSIZE;

    if ((long)(bp = mem_sbrk(size)) == -1) // 堆空间不足,分配失败
    {
        return NULL;
    }

    // bp 指向新块的首地址, 因此需要向前移动4个字节,修改前面的epilogue块
    SET(HDRP(bp), PICK(size, 0)); // 标记为未分配
    SET(FTRP(bp), PICK(size, 0));

    SET(NEXT_BLKH(bp), PICK(0, 1)); // 定义新的epilogue块

    return coalesce(bp);
}

/**
 * 根据bp的大小,
 * 返回atable对应"表"的地址, "不是"返回表指向元素的地址
 * bp不一定存在于atable中
 */
static void *get_appropriate_table(char *atable, size_t data_size)
{
    // size_t size = DATASIZE(GET_SIZE(HDRP(bp))); // 获得当前block的data size
    int i = 31; // 计算属于哪个分组

    for (i = 31; i > -1; i--)
    {
        if ((data_size >> i) & 0x1)
        {
            break;
        }
    }

    if (i >= 12)
    {
        return ATM(atable);
    }
    else
    {
        switch (i)
        {
        case 11:
            return AT4096(atable);
        case 10:
            return AT2048(atable);
        case 9:
            return AT1024(atable);
        case 8:
            return AT512(atable);
        case 7:
            return AT256(atable);
        case 6:
            return AT128(atable);
        case 5:
            return AT64(atable);
        case 4:
            return AT32(atable);
        case 3:
            return AT16(atable);
        default:
            // 最小块的数据部分大小应该为8，所以不应该出现小于8的情况
            // 这种情况不应该发生
            printf("error in coalesce\n");
            exit(1);
            break;
        }
    }
}


/**
 * 查找free block list中的某个block
*/
static void search_and_remove(char *atable, void *bp)
{
    size_t bsize = GET_SIZE(HDRP(bp));
    void *tp = get_appropriate_table(atable, DATASIZE(bsize)); // 找到对应的list
    void *p = GETDWORD(tp); // p = *tp;

    if(p == bp)
    {
        SETDWORD(tp, GET_SUCC_PTR(p));
        SET_SUCC_PTR(p, NULL);
        return;
    }

    while (p != NULL)
    {
        if (p == bp)
        {
            
            if (GET_SUCC_PTR(p) != NULL) 
            {
                // 如果存在下一个节点, 也即不是队尾
                SET_PRED_PTR(SUCC_PTR(p), GET_PRED_PTR(p)); // p->next->prev = p->prev;
                SET_SUCC_PTR(PRED_PTR(p), GET_SUCC_PTR(p)); // p->prev->next = p->next;
            }
            else
            {   
                // 如果是队尾 
                SET_SUCC_PTR(PRED_PTR(p), NULL); // p->prev->next = NULL;
            }

            SET_SUCC_PTR(p, NULL); // p->next = NULL;
            SET_PRED_PTR(p, NULL); // p->prev-NULL;
            return;
        }
        p = GET_SUCC_PTR(p); // p = p->next;
    }

    return;
}

/**
 * 合并相邻未分配的块, 减少内存碎片
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(PREV_BLKH(bp)); //  前一块是否被分配
    size_t next_alloc = GET_ALLOC(NEXT_BLKH(bp)); // 后一块是否被分配

    size_t bsize = GET_SIZE(HDRP(bp)); // 读取当前块的大小(bytes), 这里是block size

    if (prev_alloc && next_alloc)
    {
        // nothing to do 
    }
    else if (prev_alloc && !next_alloc)
    {
        bsize += GET_SIZE(NEXT_BLKH(bp));
        SET(HDRP(bp), PICK(bsize, 0));
        // 下两句等价，因为已经设置了header的size，可以通过size直接查找
        // SET(FTRP(NEXT_BLKP(bp)), PICK(bsize, 0)); 
        SET(FTRP(bp), PICK(bsize, 0));
        search_and_remove(atable, NEXT_BLKP(bp));
    }
    else if (!prev_alloc && next_alloc)
    {
        bsize += GET_SIZE(PREV_BLKH(bp));
        search_and_remove(atable, PREV_BLKP(bp));
        bp = PREV_BLKP(bp);
        SET(HDRP(bp), PICK(bsize, 0));
        SET(FTRP(bp), PICK(bsize, 0));
    }
    else
    {
        bsize += GET_SIZE(NEXT_BLKH(bp)) + GET_SIZE(PREV_BLKH(bp));
        search_and_remove(atable, NEXT_BLKP(bp));
        search_and_remove(atable, PREV_BLKP(bp));
        bp = PREV_BLKP(bp);
        SET(HDRP(bp), PICK(bsize, 0));
        SET(FTRP(bp), PICK(bsize, 0));    
    }

    void *tp = get_appropriate_table(atable, DATASIZE(bsize));
    void *p = GETDWORD(tp);

    if (p == NULL)
    {
        // 如果表是空的
        SETDWORD(tp, bp); // *tp = bp;
        SET_PRED_PTR(bp, NULL); // bp->prev = NULL;
        SET_SUCC_PTR(bp, NULL); // bp->next = NULL;
        return bp;
    }

    // p != NULL, p指向第一个free block
    SET_PRED_PTR(p, bp);    // p->prev = bp;
    SET_SUCC_PTR(bp, p);    // bp->next = p;
    SET_PRED_PTR(bp, NULL); // bp->prev = NULL;
    SETDWORD(tp, p);        // *tp = bp;

    return bp;
}

/**
 * 将输入的block进行分割，获得bsize大小的块和另一个free block
*/

static void *split_block(void *p, size_t bsize)
{
    void *bp = NULL;
    ssize_t remain_size = GET_SIZE(HDRP(p)) - bsize;
    if (remain_size > 0)
    {
        if (remain_size < 16)
        { 
            // 如果剩余部分小于最小数据块的大小(8+8)，那么整块都被分配
            search_and_remove(atable, p);
            bp = p;
        }
        else
        {
            // 如果剩余部分大于16,那么分出前一部分作为使用部分,后一部分作为free block放入list中
            search_and_remove(atable, p);
            bp = p;

            // 设置当前块的大小为 bsize
            SET(HDRP(bp), PICK(bsize, 1));
            SET(FTRP(bp), PICK(bsize, 1));

            // 设置下一块的大小为remain_size
            SET(NEXT_BLKH(bp), PICK(remain_size, 0));
            SET(FTRP(NEXT_BLKP(bp)), PICK(remain_size, 0));

            // 将free部分加入list中
            p = NEXT_BLKP(bp);
            // void *tp = get_appropriate_table(atable, DATASIZE(remain_size));

            // SET_PRED_PTR((GETDWORD(tp)), p); // ptr->prev = p;
            // SET_SUCC_PTR(p, GETDWORD(tp));   // p->next = ptr;
            // SET_PRED_PTR(p, NULL);           // p->prev = NULL;
            // SETDWORD(tp, p);                 // *tp = p;
            coalesce(p);
        }
    }
    return bp;
}