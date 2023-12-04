#define _GNU_SOURCE

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_POOL_SIZE 16
#define MAX_FD_COUNT 1024
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* struct and enum */
enum HEADER_KEYWORDS
{
    HOST,
    USER_AGENT,
    CONNECTION,
    PROXY_CONNECTION,
    OTHER_FIELDS
};

enum FILL_CTRL
{
    INIT,
    URI,
    DATA,
    FIN
};

typedef struct request_header
{
    char *value;
    struct request_header *next;
    int type;
} request_header_t;

typedef struct request
{
    char *method;
    char *host;
    char *port;
    char *uri;
    char *protocol;
    request_header_t *headers;
    request_header_t *tail;
} request_t;

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

typedef struct addrinfo addrinfo_t;

/* functions */
void *work(void *args);

/* helper */
static char *malloc_and_assign(const char *src, size_t len);
static int parse_request_line(const char *request_line, request_t *result);
static int parse_request_header(const char *request_header, request_t *req);
static int parse_http_request(int connfd, request_t *req);
static char *generate_request_message(request_t *req);
static void free_request(request_t *req);
static void init_cache(cache_t **header);

// thread_buff
static void init_thread_buff(thread_buff_t *thread_buff);
static void free_thread_buff(thread_buff_t *thread_buff);
static object_t *search_thread_buff(thread_buff_t *thread_buff, char *uri);
static void insert_object_thread_buff(thread_buff_t *thread_buff, object_t *obj);
static void fill_object(object_t **obj, int ctrl, char *data, int len);

// cache
static void init_cache(cache_t **cache);
static void free_cache(cache_t **cache);
static object_t *search_cache(cache_t **cache, char *uri);
static void insert_object_cache(cache_t **cache, object_t *obj);

/* varibles */
sem_t slots;
sem_t items;
sem_t mutex;
sem_t wlock;

int fd_pool[MAX_FD_COUNT] = {0};
int fd_index = -1;

cache_t *cache;
thread_buff_t thread_buff[MAX_POOL_SIZE];
int buff_idx = 0;
int read_count = 0;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void *work(void *args)
{
    Pthread_detach(Pthread_self());
    P(&mutex);
    thread_buff_t *buff = &thread_buff[buff_idx++];
    V(&mutex);
    request_t req;
    object_t *obj;
    while (1)
    {
        P(&items);
        P(&mutex);
        int connfd = fd_pool[fd_index--];
        V(&mutex);
        V(&slots);

        memset(&req, 0, sizeof req);

        if (parse_http_request(connfd, &req) < 0)
        {
            exit(1);
        }

        P(&mutex);
        if(++read_count == 1)
        {
            P(&wlock);
        }
        V(&mutex);
        if ((obj = search_cache(&cache, req.uri)) != NULL)
        {
            // if data in cache or buffer

            size_t remain = obj->size;
            char *pos = obj->data;
            while (remain)
            {
                size_t len = MIN(1024, remain);
                remain -= len;
                Rio_writen(connfd, pos, len);
                pos += len;
            }
            P(&mutex);
            if(--read_count == 0)
            {
                V(&wlock);
            }
            V(&mutex);
            continue;
        }

        P(&mutex);
        if (--read_count == 0)
        {
            V(&wlock);
        }
        V(&mutex);

        char *message = generate_request_message(&req);
        char *reply = (char *)malloc(sizeof(char) * MAXLINE);
        size_t maxlen = MAXLINE, len = 0;
        int clientfd = Open_clientfd(req.host, req.port);

        rio_t rio;
        Rio_readinitb(&rio, clientfd);
        Rio_writen(clientfd, (void *)message, strlen(message));

        obj = malloc(sizeof(object_t));
        memset(obj, 0, sizeof(object_t));

        fill_object(&obj, INIT, NULL, 0);
        fill_object(&obj, URI, req.uri, strlen(req.uri));
        while ((len = Rio_readlineb(&rio, reply, maxlen)) > 0)
        {
            Rio_writen(connfd, reply, len);
            fill_object(&obj, DATA, reply, len);
        }
        fill_object(&obj, FIN, NULL, 0);
        P(&wlock);
        insert_object_thread_buff(buff, obj);
        V(&wlock);
        free_request(&req);
        free(message);
        free(reply);
        Close(connfd);
    }
}

/* main */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "useage %s <port>\n", argv[0]);
        exit(1);
    }
    int listenfd = Open_listenfd(argv[1]);
    int connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[48], client_port[48];

    init_cache(&cache);
    init_thread_buff(thread_buff);
    Sem_init(&slots, 0, MAX_POOL_SIZE);
    Sem_init(&items, 0, 0);
    Sem_init(&mutex, 0, 1);
    Sem_init(&wlock, 0, 1);


    pthread_t threads[MAX_POOL_SIZE];

    for (int i = 0; i < MAX_POOL_SIZE; i++)
    {
        Pthread_create(&threads[i], NULL, work, NULL);
    }

    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, 48, client_port, 48, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        P(&slots);
        P(&mutex);
        fd_pool[++fd_index] = connfd;
        V(&mutex);
        V(&items);
    }
    free_cache(&cache);
    free_thread_buff(thread_buff);
    close(listenfd);
    return 0;
}

char *malloc_and_assign(const char *src, size_t len)
{
    char *dst = (char *)malloc(sizeof(char) * (len + 1));
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

int parse_http_request(int connfd, request_t *req)
{
    char buff[MAXLINE];
    rio_t rio;
    int ret;
    ssize_t len = 0;
    Rio_readinitb(&rio, connfd);
    if ((len = Rio_readlineb(&rio, buff, MAXLINE)) != 0)
    {
        ret = parse_request_line(buff, req);
        if (ret != 0)
        {
            return -1;
        }
    }

    while ((len = Rio_readlineb(&rio, buff, MAXLINE)) != 0)
    {
        ret = parse_request_header(buff, req);
        switch (ret)
        {
        case 0:
            continue;
        case 1:
            break;
        case -1:
        default:
            return -1;
        }
        break;
    }
    return 0;
}

int parse_request_line(const char *request_line, request_t *result)
{
    const char *pos, *end, *delim;
    size_t len = strlen(request_line);
    pos = request_line;

    result->method = malloc_and_assign("GET", 3);

    if ((pos = strchr(pos, ' ')) != NULL && (end = strchr(++pos, ' ')) != NULL)
    {
        size_t hostlen = 0;
        // skip "http://"
        if (strstr(pos, "http"))
        {
            pos += 7;
        }

        // find root '/'
        if ((delim = strchr(pos, '/')) != NULL)
        {
            hostlen = delim - pos;
        }

        char *host_port = malloc_and_assign(pos, hostlen);
        pos = delim;

        if ((delim = strchr(host_port, ':')) != NULL)
        {
            // if input host with port
            delim++;
            size_t portlen = hostlen - (delim - host_port);
            hostlen -= portlen + 1;

            result->host = malloc_and_assign(host_port, hostlen);
            result->port = malloc_and_assign(delim, portlen);

            free(host_port);
        }
        else
        {
            result->host = host_port;
            result->port = malloc_and_assign("80", 3); // use default port
        }

        size_t urilen = end - pos;
        result->uri = malloc_and_assign(pos, urilen);

        end++;
        size_t protolen = len - (end - request_line);
        result->protocol = malloc_and_assign(end, protolen);
    }
    else
    {
        fprintf(stderr, "error in parse_request_line\n");
        return -1;
    }

    return 0;
}

int parse_request_header(const char *request_header, request_t *req)
{
    size_t len = strlen(request_header);

    if (len < 2)
    {
        printf("error in parse_requset_header.\n");
        return -1;
    }

    if (len == 2)
    {
        // read "\r\n" blank line, stop read
        return 1;
    }

    int type = OTHER_FIELDS;

    if (strstr(request_header, "Host"))
    {
        type = HOST;
    }
    else if (strstr(request_header, "User-Agent"))
    {
        type = USER_AGENT;
    }
    else if (strstr(request_header, "Connection"))
    {
        type = CONNECTION;
    }
    else if (strstr(request_header, "Proxy-Connection"))
    {
        type = PROXY_CONNECTION;
    }

    if (req->headers == NULL)
    {
        req->headers = (request_header_t *)malloc(sizeof(request_header_t));
        req->tail = req->headers;
    }
    else
    {
        req->tail->next = (request_header_t *)malloc(sizeof(request_header_t));
        req->tail = req->tail->next;
    }

    req->tail->type = type;
    req->tail->value = malloc_and_assign(request_header, len - 2);
    req->tail->next = NULL;

    return 0;
}

char *generate_request_message(request_t *req)
{
    char *message = (char *)malloc(sizeof(char) * MAXLINE);
    char *msg = message;
    msg += sprintf(msg, "%s %s HTTP/1.0\r\n", req->method, req->uri);
    msg += sprintf(msg, "Host: %s:%s\r\n", req->host, req->port);
    msg += sprintf(msg, "%s", user_agent_hdr); // user-agent
    msg += sprintf(msg, "Connection: close\r\n");
    msg += sprintf(msg, "Proxy-Connection: close\r\n");
    request_header_t *hp = req->headers, *p;

    while (hp != NULL)
    {
        p = hp->next;
        if (hp->type == OTHER_FIELDS)
        {
            msg += sprintf(msg, "%s\r\n", hp->value);
        }
        hp = p;
    }
    sprintf(msg, "\r\n");
    message = (char *)realloc(message, strlen(message) + 1);
    return message;
}

void free_request(request_t *req)
{
    free(req->host);
    free(req->method);
    free(req->port);
    free(req->protocol);
    free(req->uri);
    request_header_t *hp = req->headers, *p;
    while (hp != NULL)
    {
        p = hp->next;
        free(hp->value);
        free(hp);
        hp = p;
    }
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
        while (p->next != NULL)
        {
            if (strcmp(p->uri, uri) == 0)
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
    while (p->next != NULL)
    {   
        if (strcmp(p->uri, uri) == 0)
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
        return q;
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

void fill_object(object_t **obj, int ctrl, char *data, int len)
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
        o->uri = (char *)realloc(o->uri, len + 1);
        break;
    case DATA:
        memcpy(pos, data, len);
        o->size += len;
        break;
    case FIN:
        o->data = (char *)realloc(o->data, o->size);
        break;
    default:
        break;
    }
}
