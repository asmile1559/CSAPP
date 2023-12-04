#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 8192

enum HEADER_KEYWORDS
{
    HOST,
    USER_AGENT,
    CONNECTION,
    PROXY_CONNECTION,
    OTHER_FIELDS
};

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

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

char *malloc_and_assign(const char *src, size_t len)
{
    char *dst = (char *)malloc(sizeof(char) * (len + 1));
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

// support GET request
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
        size_t protolen = len - (end - request_line) - 2;
        result->protocol = malloc_and_assign(end, protolen);
    }
    else
    {
        fprintf(stderr, "error GET format\n");
        return -1;
    }

    // printf("host: %s\nport: %s\nuri: %s\nprotocol: %s\n", result->host, result->port, result->uri, result->protocol);
    return 0;
}

int parse_request_header(const char *request_header, request_t *req)
{
    size_t len = strlen(request_header);

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
    size_t len;
    msg += sprintf(msg, "%s http://%s:%s%s HTTP/1.0\r\n", req->method, req->host, req->port, req->uri);
    msg += sprintf(msg, "Host: %s:%s\r\n", req->host, req->port);
    msg += sprintf(msg, "%s", user_agent_hdr); // user-agent
    msg += sprintf(msg, "Connection: close\r\n");
    msg += sprintf(msg, "Proxy-Connection: close\r\n");
    request_header_t *hp = req->headers, *p;

    while (hp != NULL)
    {
        p = hp->next;
        if(hp->type == OTHER_FIELDS)
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

int main()
{
    const char *s = "GET http://localhost:15213/home/xd20/cmu15213/proxylab-handout/tiny/home.html HTTP/1.1\r\n";
    const char *a = "Host: localhost:15213\r\n";
    const char *b = "User-Agent: curl/7.68.0\r\n";
    const char *c = "Accept: */*\r\n";
    const char *d = "Proxy-Connection: Keep-Alive\r\n";
    const char *e = "\r\n";
    request_t req;
    memset(&req, 0, sizeof(req));
    parse_request_line(s, &req);
    parse_request_header(a, &req);
    parse_request_header(b, &req);
    parse_request_header(c, &req);
    parse_request_header(d, &req);
    parse_request_header(e, &req);
    char *msg = generate_request_message(&req);
    printf("%s", msg);
    free(msg);
    free_request(&req);

    // parse_request_line(s, &req);
    // printf("host: %s\nport: %s\nuri: %s\nprotocol: %s\n", req.host, req.port, req.uri, req.protocol);
    // free(req.host);
    // free(req.method);
    // free(req.protocol);
    // free(req.uri);
    // free(req.port);
}