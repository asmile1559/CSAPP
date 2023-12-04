#include "csapp.h"
#include <unistd.h>

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_POOL_SIZE 16

enum FILL_CTRL
{
    INIT,
    URI,
    DATA,
    FIN
};

typedef struct object
{
    char *uri;
    size_t size;
    char *data;
    struct object *prev;
    struct object *next;
} object_t;

typedef struct thread_buff
{
    size_t remain_size;
    object_t *head;

} thread_buff_t;

typedef struct cache
{
    object_t *head;
    object_t *tail;
    size_t remain_size;
} cache_t;

void init_thread_buff(thread_buff_t *thread_buff);
void free_thread_buff(thread_buff_t *thread_buff);

void init_cache(cache_t **cache);
void free_cache(cache_t **cache);

object_t *search_thread_buff(thread_buff_t *thread_buff, char *uri);
object_t *search_cache(cache_t **cache, char *uri);

// "thread_buff" is not an array here.
void insert_object_thread_buff(thread_buff_t *thread_buff, object_t *obj);
void insert_object_cache(cache_t **cache, object_t *obj);

void fill_object(object_t **obj, int ctrl, char *data);

cache_t *cache;
thread_buff_t thread_buff[MAX_POOL_SIZE];
sem_t slots;
sem_t items;
sem_t mutex;
int buff_idx = 0;

void *work(void *args)
{
    int i = 1;
    Pthread_detach(Pthread_self());
    P(&mutex);
    thread_buff_t *buff = &thread_buff[buff_idx++];
    V(&mutex);
    object_t *obj;
    char data[160];

    if ((obj = search_cache(&cache, data)) != NULL)
    {
        printf("%s", obj->data);
    }
    else
    {
        obj = malloc(sizeof(object_t));
        memset(obj, 0, sizeof(object_t));

        fill_object(&obj, INIT, NULL);
        sprintf(data, "%s", "home.html");
        fill_object(&obj, URI, data);

        while (i--)
        {
            sprintf(data, "%ld", Pthread_self());
            fill_object(&obj, DATA, data);
        }
        fill_object(&obj, FIN, data);
        insert_object_thread_buff(buff, obj);
    }
}

int main()
{
    Sem_init(&slots, 0, MAX_POOL_SIZE);
    Sem_init(&items, 0, 0);
    Sem_init(&mutex, 0, 1);

    init_cache(&cache);
    init_thread_buff(thread_buff);

    pthread_t tid;

    for (int i = 0; i < MAX_POOL_SIZE; i++)
    {
        Pthread_create(&tid, NULL, work, NULL);
    }

    free_cache(&cache);
    free_thread_buff(thread_buff);
}

void init_thread_buff(thread_buff_t *thread_buff)
{
    size_t size_buff = sizeof(thread_buff_t);
    size_t size_obj = sizeof(object_t);
    for (int i = 0; i < MAX_POOL_SIZE; i++)
    {
        memset(&thread_buff[i], 0, size_buff);
        thread_buff[i].remain_size = MAX_OBJECT_SIZE;
        thread_buff[i].head = (object_t *)malloc(size_obj);
        memset(thread_buff[i].head, 0, size_obj);
        (thread_buff[i].head)->next = (object_t *)malloc(size_obj);
        memset((thread_buff[i].head)->next, 0, size_obj);
    }
    return;
}

object_t *search_thread_buff(thread_buff_t *thread_buff, char *uri)
{
    for (int i = 0; i < MAX_POOL_SIZE; i++)
    {
        object_t *p = thread_buff[i].head->next;
        while (p != NULL)
        {
            if (strcmp(p->uri, uri))
            {
                // remove from thread_buff
                p->prev->next = p->next;
                p->next->prev = p->prev;
                p->prev = NULL;
                p->next = NULL;
                thread_buff[i].remain_size += p->size;
                return p;
            }
            p = p->next;
        }
    }
    return NULL;
}

object_t *search_cache(cache_t **cache, char *uri)
{
    object_t *p = (*cache)->head->next, *q;
    while (p != NULL)
    {
        if (strcmp(p->uri, uri))
        {
            p->prev->next = p->next;
            p->next->prev = p->prev;

            p->next = (*cache)->head->next;
            p->next->prev = p;

            (*cache)->head->next = p;
            p->prev = (*cache)->head;
            return p;
        }
        p = p->next;
    }

    // cant find obj in cache
    if ((q = search_thread_buff(thread_buff, uri)) != NULL)
    {
        insert_object_cache(cache, q);
    }
    return NULL;
}

// "thread_buff" is not an array here.
void insert_object_thread_buff(thread_buff_t *thread_buff, object_t *obj)
{
    if (obj->size > MAX_OBJECT_SIZE)
    {
        return;
    }

    if (thread_buff->remain_size > obj->size)
    {
        obj->next = thread_buff->head->next;
        obj->prev = thread_buff->head;

        thread_buff->head->next = obj;
        obj->next->prev = obj;
        thread_buff->remain_size -= obj->size;
    }
    else
    {
        object_t *p = thread_buff->head->next, *q;
        while (p != NULL)
        {
            q = p->next;
            free(p->data);
            free(p->uri);
            free(p);
            p = q;
        }
        thread_buff->remain_size = MAX_OBJECT_SIZE;
        insert_object_thread_buff(thread_buff, obj);
    }
}

void insert_object_cache(cache_t **cache, object_t *obj)
{
    if ((*cache)->remain_size > obj->size)
    {
        obj->next = (*cache)->head->next;
        obj->next->prev = obj;

        (*cache)->head->next = obj;
        obj->prev = (*cache)->head;

        (*cache)->remain_size -= obj->size;
    }
    else
    {
        object_t *p = (*cache)->tail->prev;

        p->prev->next = (*cache)->tail;
        (*cache)->tail->prev = p->prev;

        (*cache)->remain_size += p->size;
        free(p);
        insert_object_cache(cache, obj);
    }
}

void init_cache(cache_t **cache)
{
    size_t cache_size = sizeof(cache_t);
    size_t object_size = sizeof(object_t);

    *cache = (cache_t *)malloc(cache_size);
    (*cache)->head = (object_t *)malloc(object_size);
    (*cache)->tail = (object_t *)malloc(object_size);

    memset((*cache)->head, 0, object_size);
    memset((*cache)->tail, 0, object_size);

    (*cache)->head->next = (*cache)->tail;
    (*cache)->tail->prev = (*cache)->head;
    (*cache)->remain_size = MAX_CACHE_SIZE;

    return;
}

void free_thread_buff(thread_buff_t *thread_buff)
{
    for (int i = 0; i < MAX_POOL_SIZE; i++)
    {
        object_t *p = thread_buff[i].head, *q;
        while (p != NULL)
        {
            q = p->next;
            if (p->data)
                free(p->data);
            if (p->uri)
                free(p->uri);
            free(p);
            p = q;
        }
    }
}

void free_cache(cache_t **cache)
{
    object_t *head = (*cache)->head;
    object_t *tail = (*cache)->tail;
    object_t *p = head->next;

    while (p != tail)
    {
        if (p->data)
            free(p->data);
        if (p->uri)
            free(p->uri);
        p = p->next;
    }

    free(head);
    free(tail);
    free(*cache);

    return;
}

void fill_object(object_t **obj, int ctrl, char *data)
{
    object_t *o = *obj;
    char *pos = o->data + o->size;
    switch (ctrl)
    {
    case INIT:
        o->size = 0;
        o->uri = (char *)malloc(sizeof(char) * MAXLINE);
        o->data = (char *)malloc(sizeof(char) * MAX_OBJECT_SIZE);
        break;
    case URI:
        strcpy(o->uri, data);
        o->uri = (char *)realloc(o->uri, strlen(data) + 1);
        break;
    case DATA:
        o->size += sprintf(pos, "%s", data);
        break;
    case FIN:
        o->data = (char *)realloc(o->data, o->size + 1);
        break;
    default:
        break;
    }
}
