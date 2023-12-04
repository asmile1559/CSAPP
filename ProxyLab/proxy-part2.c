#define _GNU_SOURCE

#include "csapp.h"

enum HEADER_KEYWORDS
{
    HOST,
    USER_AGENT,
    CONNECTION,
    PROXY_CONNECTION,
    OTHER_FIELDS
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

typedef struct addrinfo addrinfo_t;

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAX_POOL_SIZE 16
#define MAX_FD_COUNT 1024

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* functions */
int work(int connfd);
void *thread(void *args);

/* helper */
static char *malloc_and_assign(const char *src, size_t len);
static int parse_request_line(const char *request_line, request_t *result);
static int parse_request_header(const char *request_header, request_t *req);
static int parse_http_request(int connfd, request_t *req);
static char *generate_request_message(request_t *req);
static void free_request(request_t *req);

/* varibles */
sem_t slots;
sem_t items;
sem_t mutex;
int fd_pool[MAX_FD_COUNT] = {0};
int fd_index = -1;

void *thread(void *args)
{
    Pthread_detach(Pthread_self());
    while (1)
    {
        P(&items);
        P(&mutex);
        int connfd = fd_pool[fd_index--];
        printf("read from fd_pool, connect fd = %d, pthread id = %ld\n", connfd, Pthread_self());
        V(&mutex);
        V(&slots);
        if(work(connfd) != 0)
        {
            exit(1);
        }
        Close(connfd);
    }
}

/* main */
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "useage %s <port>\n", argv[0]);
        exit(1);
    }
    int listenfd = Open_listenfd(argv[1]);
    int connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[48], client_port[48];

    Sem_init(&slots, 0, MAX_POOL_SIZE);
    Sem_init(&items, 0, 0);
    Sem_init(&mutex, 0, 1);
    pthread_t threads[MAX_POOL_SIZE];

    for (int i = 0; i < MAX_POOL_SIZE; i++)
    {
        Pthread_create(&threads[i], NULL, thread, NULL);
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

int work(int connfd)
{
    request_t req;
    memset(&req, 0, sizeof req);
    if (parse_http_request(connfd, &req) < 0)
    {
        return -1;
    }

    char *message = generate_request_message(&req);
    char *reply = (char *)malloc(sizeof(char) * MAXLINE);
    size_t maxlen = MAXLINE, len = 0;
    int clientfd = Open_clientfd(req.host, req.port);
    
    rio_t rio;
    Rio_readinitb(&rio, clientfd);
    Rio_writen(clientfd, (void *)message, strlen(message));

    while ((len = Rio_readlineb(&rio, reply, maxlen)) > 0)
    {
        Rio_writen(connfd, reply, len);
    }

    free_request(&req);
    free(message);
    free(reply);
    return 0;
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

    if(len < 2)
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
