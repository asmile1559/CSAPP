#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

/* macro define */
#define BUF_SIZE 20

/* struct define */
typedef struct Block
{
    // Block 应该是一个整体,这个整体共同参与数据的使用
    // 换入&换出的单位就是Block
    uint64_t tag;       // tag位
    uint8_t *data;      // 存储的是数据
    uint8_t vaild;      // 有效标志位
    struct Block *prev; // 指向上一个block块
    struct Block *next; // 指向下一个block块
} Block;

typedef struct Section
{
    // Section应该设计为LRU结构, 在Section中进行换入换出操作， 换的是Block块
    uint64_t e_count;  // block的数量, e路
    Block *block_head; // 指向Block头部
    Block *block_tail; // 指向Block尾部
} Section;

typedef struct Cache
{
    // Cache是外部的宏观结构, 应该负责控制Section的头尾指针
    uint64_t s_count;  // section 的数量, s组
    Section *sections; // 缓存数据
} Cache;

typedef struct Splited_Addr
{
    uint64_t tag;
    uint64_t set;
    uint64_t bias;
} spa_t;

typedef enum Options
{
    LOAD,
    MODIFY,
    STORE,
    ILLEGAL
} opt_t;

/* functions declare*/
int decstr2int(char *str);
void parse_args(int argc, char *argv[], int vsEb[5], char **trace);

void print_help();
Cache *init_cache(int s, int E, int b);
void delete_cache(Cache *cache);
opt_t parse_trace_log(char *log, uint64_t *addr, uint32_t *size);

void split_addr(uint64_t addr, int s, int b, spa_t *spa);
Block *search(Cache *cache, spa_t *spa);

// main
int main(int argc, char *argv[])
{
    char buff[BUF_SIZE]; // buff for fgets
    int hit_count = 0, evict_count = 0, miss_count = 0;

    int vsEb[4] = {0}; // vsEb: 0->v, 1->s, 2->E, 3->b
    char *trace = NULL;
    parse_args(argc, argv, vsEb, &trace);

    Cache *cache = init_cache(vsEb[1], vsEb[2], vsEb[3]);

    FILE *fp = fopen(trace, "r");
    assert(fp);
    opt_t t = ILLEGAL;
    int process = 1;
    while (fgets(buff, BUF_SIZE, fp) != NULL)
    {
        uint32_t size = 0;
        uint64_t addr = 0;
        t = parse_trace_log(buff, &addr, &size);

        if (t == ILLEGAL)
        {
            memset(buff, 0, BUF_SIZE);
            continue;
        }

        spa_t spa;
        split_addr(addr, vsEb[1], vsEb[3], &spa);
        Block *p = search(cache, &spa);

        // LRU
        Section *sec = &cache->sections[spa.set];
        Block *r = NULL;

        if (p == sec->block_tail)
        {
            r = p->prev;
        }
        else
        {
            r = p;
        }

        r->prev->next = r->next;
        r->next->prev = r->prev;

        r->next = sec->block_head->next;
        r->prev = sec->block_head;

        r->next->prev = r;
        r->prev->next = r;

        char *desc = NULL;
        if (p->next == NULL)
        {
            miss_count++;
            if (t == MODIFY)
            {
                hit_count++;
                if (r->vaild != 0)
                {
                    evict_count++;
                    desc = "miss eviction hit";
                }
                else
                    desc = "miss hit";
            }
            else
            {
                if (r->vaild != 0)
                {
                    evict_count++;
                    desc = "miss eviction";
                }
                else
                {
                    desc = "miss";
                }
            }
            r->vaild = 1;
            r->tag = spa.tag;
        }
        else
        {
            if (t == MODIFY)
            {
                hit_count += 2;
                desc = "hit hit";
            }
            else
            {
                hit_count++;
                desc = "hit";
            }
        }

        if (vsEb[0])
        {
            char c;
            switch (t)
            {
            case LOAD:
                c = 'L';
                break;
            case MODIFY:
                c = 'M';
                break;
            case STORE:
                c = 'S';
                break;
            default:
                continue;
            }
            printf("%c %lx,%d %s\n", c, addr, size, desc);
        }
        memset(buff, 0, BUF_SIZE);
        process++;
    }

    fclose(fp);
    delete_cache(cache);

    printSummary(hit_count, miss_count, evict_count);
    return 0;
}

int decstr2int(char *str)
{
    int num = 0, i = 0;
    while (str[i])
    {
        num *= 10;
        if ('0' > str[i] || '9' < str[i])
        {
            return -1;
        }
        num += (str[i] - '0');
        i++;
    }
    return num;
}

void parse_args(int argc, char *argv[], int vsEb[5], char **trace)
{
    for (int i = 1; i < argc; i++)
    {
        char *cmd = argv[i];
        if (cmd[0] == '-')
        {
            switch (cmd[1])
            {
            case 'h':
                print_help();
                exit(0);
            case 'v':
                vsEb[0] = 1;
                break;
            case 's':
                vsEb[1] = decstr2int(argv[++i]);
                break;
            case 'E':
                vsEb[2] = decstr2int(argv[++i]);
                break;
            case 'b':
                vsEb[3] = decstr2int(argv[++i]);
                break;
            case 't':
                *trace = argv[++i];
                break;
            default:
                printf("./csim: invalid option -- '%c'\n", cmd[1]);
                print_help();
                exit(0);
            }
        }

        for (int i = 1; i < 5; i++)
        {
            if (vsEb[i] == -1)
            {
                printf("./csim: Missing required command line argument\n");
                print_help();
                exit(0);
            }
        }
    }
    return;
}

void print_help()
{
    printf("%s",
           "Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n"
           "Options:\n"
           "  -h         Print this help message.\n"
           "  -v         Optional verbose vsEb.\n"
           "  -s <num>   Number of set index bits.\n"
           "  -E <num>   Number of lines per set.\n"
           "  -b <num>   Number of block offset bits.\n"
           "  -t <file>  Trace file.\n"
           "\n"
           "Examples:\n"
           "  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n"
           "  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
    return;
}

Cache *init_cache(int s, int E, int b)
{
    int S = 1 << s, B = 1 << b;

    Cache *cache = (Cache *)malloc(sizeof(Cache));
    cache->s_count = S;
    cache->sections = (Section *)malloc(sizeof(Section) * S);

    Section *sec;
    for (int i = 0; i < S; i++)
    {
        sec = cache->sections + i;
        sec->e_count = E;
        sec->block_head = (Block *)malloc(sizeof(Block));

        Block *p = NULL, *q = NULL;
        p = sec->block_head;
        p->prev = NULL;
        for (int j = 0; j <= E; j++)
        {
            q = (Block *)malloc(sizeof(Block));
            memset(q, 0, sizeof(*q));
            q->tag = -1;
            if (j != E)
                q->data = (uint8_t *)malloc(sizeof(uint8_t) * B);

            p->next = q;
            q->prev = p;
            p = q;
        }

        p->next = NULL;
        sec->block_tail = p;
    }
    return cache;
}

void delete_cache(Cache *cache)
{
    int s_count = cache->s_count;
    for (int i = 0; i < s_count; i++)
    {
        Section *sec = cache->sections + i;
        int e_count = sec->e_count;

        Block *p = NULL, *q = NULL;
        p = sec->block_head->next;
        q = p->next;
        for (int j = 0; j < e_count; j++)
        {
            free(p->data);
            free(p);
            p = q;
            q = q->next;
        }

        free(sec->block_head);
        free(sec->block_tail);
    }
    free(cache->sections);
    free(cache);
    return;
}

opt_t parse_trace_log(char *log, uint64_t *addr, uint32_t *size)
{
    if (log[0] != ' ')
    {
        *addr = 0;
        *size = 0;
        return ILLEGAL;
    }

    int len = strlen(log);
    int i = 3;
    *addr = 0;

    for (; i < len; i++)
    {
        char c = log[i];
        if (c == ',')
            break;
        *addr *= 16;
        if ('0' <= c && c <= '9')
        {
            *addr += c - '0';
        }
        else
        {
            *addr += c - 'a' + 10;
        }
    }

    for (i++; i < len; i++)
    {
        if (log[i] < '0' || log[i] > '9')
            continue;
        *size *= 10;
        *size += log[i] - '0';
    }

    switch (log[1])
    {
    case 'L':
        return LOAD;
    case 'M':
        return MODIFY;
    case 'S':
        return STORE;
    default:
        *addr = 0;
        *size = 0;
        return ILLEGAL;
    }
}

void split_addr(uint64_t addr, int s, int b, spa_t *spa)
{
    int t = 64 - s - b;
    spa->bias = (addr << (64 - b)) >> (64 - b);
    spa->set = (((addr << t) >> t) >> b);
    spa->tag = addr >> (s + b);
}

/**
 * return Block *ptr;
 * 1. if find the tag -> ptr != sec->block_tail;
 * 2. if not find the tag -> ptr->next == sec->block_tail -> ptr->next = NULL;
 */
Block *search(Cache *cache, spa_t *spa)
{
    Section *sec = &cache->sections[spa->set];
    int e = sec->e_count;
    Block *p = sec->block_head->next;

    for (int i = 0; i < e; i++)
    {
        if (p->tag == spa->tag)
        {
            return p;
        }
        else
        {
            p = p->next;
        }
    }
    return p;
}