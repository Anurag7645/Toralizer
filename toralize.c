/* toralize.c */

/* 

1. Turn the client into a lib (shared library) .so
2. Turn main() into our own connect()
3. Replace regular connect()
4. Grab the ip and port from original connect()

*/

#include "toralize.h"

Req *request(struct sockaddr_in *sock2) 
{
    Req *req;

    req = malloc(reqsize);

    req->vn = 4;
    req->cd = 1;
    req->dstport = sock2->sin_port;
    req->dstip = sock2->sin_addr.s_addr;
    strncpy(req->userid, USERNAME, 8);

    return req;

}

/*int main(int argc, char *argv[])*/ 
int connect(int s2, const struct sockaddr *sock2,socklen_t addrlen)
{
    // char *host;
    int s;
    struct sockaddr_in sock;
    Req *req;
    Res *res;
    char buf[ressize];
    int success;
    char tmp[512];
    int (*p)(int, const struct sockaddr*, socklen_t);

    /* predicate - property*/

    // if (argc < 3)
    // {
    //     fprintf(stderr, "Usage: %s <host> <port>\n",
    //     argv[0]);

    //     return -1;
    // }

    // host = argv[1];
    // port = atoi(argv[2]);
    p = dlsym(RTLD_NEXT, "connect");

    s =socket(AF_INET, SOCK_STREAM, 0);
    if (s<0) 
    {
        perror("socket");

        return -1;
    }

    sock.sin_family = AF_INET;
    sock.sin_port = htons(PROXYPORT);
    sock.sin_addr.s_addr = inet_addr(PROXY);

    if (p(s, (struct sockaddr *)&sock, sizeof(sock)))
    {
        perror("connect");

        return -1;
    }

    printf("Connected to proxy\n");
    req = request((struct sockaddr_in *)sock2);
    write(s, req, reqsize);

    memset(buf, 0, ressize);
    if (read(s, buf, ressize) < 1)
    {
        perror("read");
        free(req);
        close(s);

        return -1;
    }

    res = (Res *)buf;
    success = (res->cd == 90);
    if (!success)
    {
        fprintf(stderr, "Unabel to traverse the proxy, error code: %d\n",res->cd);

        close(s);
        free(req);

        return -1;
    }

    printf("Successfully connected through the proxy to\n");
    
    dup2(s,s2);

    // memset(tmp, 0, 512);
    // snprintf(tmp, 511,
    //     "HEAD / HTTP/1.0\r\n"
    //     "Host: www.networktechnology.org\r\n"
    //     "\r\n"
    // );
    // write(s, tmp, strlen(tmp));

    // memset(tmp, 0, 512);
    // read(s, tmp, 511);
    // printf("'%s'\n", tmp);

    // close(s);

    free(req);
    
    return 0;

}