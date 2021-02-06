/* 参考：
https://blog.csdn.net/qq_29611575/article/details/102688505?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522161243577316780269886406%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=161243577316780269886406&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~top_click~default-1-102688505.pc_search_result_no_baidu_js&utm_term=proxy+lab&spm=1018.2226.3001.4187
https://blog.csdn.net/a2888409/article/details/47186725?ops_request_misc=&request_id=&biz_id=102&utm_term=proxy%20lab&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-1-47186725.pc_search_result_no_baidu_js&spm=1018.2226.3001.4187
 */
#include <stdio.h>
#include "csapp.h"
#include "cache.h"
#include "sbuf.h"

#define IS_HEAD_METHOD  1
#define IS_POST_METHOD  2
#define IS_GET_METHOD   3


void doit(int fd);
void read_requesthdrs(rio_t *rp);
void parse_uri(char* url, char* hostname, char* path, char* port);
void serve_static(int fd, char *filename, int filesize, int method);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, int method, rio_t *rp);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);
void sigepipe_handler(int sig);

int connect_server(char* host_ip, char* port, char* query_path);//连接服务器并转发请求，成功则返回服务器的套接字描述符，失败返回0
void *thread();

cache_t cache;  /* Shared cache */
sbuf_t threads_pool;

int main(int argc, char **argv)
{
    int listenfd, clientlen, connfd;
    char* port;
    struct sockaddr_in clientaddr;
    pthread_t tid;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    //初始化cache和线程池
    cache_init(&cache);
    sbuf_init(&threads_pool, SBUFSIZE);

    for (int i = 0; i < NTHREADS; i++) 
        Pthread_create(&tid, NULL, thread, NULL);

    // port = atoi(argv[1]);
    port = argv[1];

    /* csapp 11.13 */
    if (Signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        unix_error("mask signal pipe error");
    }

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        sbuf_insert(&threads_pool, connfd);
    }
}
/* $end tinymain */

/* $begin thread */
void *thread() {
    Pthread_detach(pthread_self());
    while (1) {
        int client_fd = sbuf_remove(&threads_pool);
        doit(client_fd);
        Close(client_fd);
    }
}
/* $end thread */

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int client_fd)
{
    int which_method;
    int server_fd;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], filename[MAXLINE], hostname[MAXLINE], port[10];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, client_fd);
    Rio_readlineb(&rio, buf, MAXLINE);                   //line:netp:doit:readrequest
    sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest

    /* csapp 11.11 */
    if (strcasecmp(method, "GET") == 0) {                     //line:netp:doit:beginrequesterr
        which_method = IS_GET_METHOD;
    } else if (strcasecmp(method, "HEAD") == 0) {
        which_method = IS_HEAD_METHOD;
    } else if (strcasecmp(method, "POST") == 0) {
        which_method = IS_POST_METHOD;
    } else {
        clienterror(client_fd, method, "501", "Not Implemented",
            "Tiny does not implement this method");
        return;
    }
    
    //忽略首部和实体
    read_requesthdrs(&rio);                              //line:netp:doit:readrequesthdrs

    /* find cache */
    int cache_index;
    if ((cache_index = cache_find(&cache, uri)) != -1) {
        Rio_writen(client_fd, cache.cache_objects[cache_index].obj, strlen(cache.cache_objects[cache_index].obj));
        read_after(&cache, cache_index);
        return;
    }

    /* Parse URI from GET request */
    parse_uri(uri, hostname, filename, port);       //line:netp:doit:staticcheck

    //try to connect server
    server_fd = connect_server(hostname, port, filename);
    if (server_fd == 0) {
        return;
    }
    //等待服务器返回结果，然后转发给客户端
    char cache_buf[MAX_OBJECT_SIZE];
    int n, buf_size = 0;
    printf("Get response from server, now try to forward to client...\n\n");
    Rio_readinitb(&rio, server_fd);
    while (n = Rio_readlineb(&rio, buf, MAXLINE)) {

        buf_size += n;
        if (buf_size < MAX_OBJECT_SIZE) {
            strcat(cache_buf, buf);
        }

        Rio_writen(client_fd, buf, n);
    }
    
    //关闭客户端、服务器的连接
    printf("Finish, now close the connection.\n\n");
    Close(server_fd);

    /* store it in cache */
    if (n < MAX_OBJECT_SIZE) {
        cache_store(&cache, uri, cache_buf);
    }
}
/* $end doit */

/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
//@todo: 异常输入的处理
void parse_uri(char* url, char* hostname, char* query_path, char* port)
{
    //拷贝一份URL，避免在函数内被"污染"
    char url_backup[100];
    int port_num;
    url_backup[0] = '\0';
    strcat(url_backup, url);

    //在URL中分割出hostname、path
    //首先找到‘//‘的下一位
    //然后在这一位的基础上，找到‘:‘,并将这里改成'\0'，那么scanf函数的%s参数可以识别出hostname
    char *p = strstr(url_backup, "//");
    p += 2;
    char  *q = strstr(p, ":");
    *q = '\0';
    sscanf(p, "%s", hostname);
    sscanf(q + 1, "%d%s", &port_num, query_path);
    sprintf(port, "%d", port_num);
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client 
 */
/* $begin serve_static */

void serve_static(int fd, char *filename, int filesize, int method)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);       //line:netp:servestatic:getfiletype
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));       //line:netp:servestatic:endserve

    /* csapp 11.11 */
    if (method == IS_HEAD_METHOD) return;

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);    //line:netp:servestatic:open

    /* csapp 11.9 */
    srcp = (char*)malloc(filesize);
    Rio_readn(srcfd, srcp, filesize);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);         //line:netp:servestatic:write
    free(srcp);
    // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);//line:netp:servestatic:mmap
    // Close(srcfd);                           //line:netp:servestatic:close
    // Rio_writen(fd, srcp, filesize);         //line:netp:servestatic:write
    // Munmap(srcp, filesize);                 //line:netp:servestatic:munmap
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");

    /** csapp 11.7 **/
    else if (strstr(filename, "./mpg"))
        strcpy(filetype, "video/mpg");
    /** end **/

    else
        strcpy(filetype, "text/plain");
}
/* $end serve_static */

/** handler sigchild **/
void sigchild_handler(int sig) {
    int old_errno = errno;
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status,WNOHANG | WUNTRACED)) > 0) {
        printf("PID(%d) Stopped by signal %d\n", pid, WSTOPSIG(status));
    }
    errno = old_errno;
}

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs, int method, rio_t *rp)
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* csapp 11.11 */
    if (method == IS_HEAD_METHOD) return;

    /* csapp 11.12 */
    /* question:万一post的东西很大怎么办？ */
    if (method == IS_POST_METHOD){  /* POST */
        Rio_readnb(rp, buf, rp->rio_cnt);
        strcpy(cgiargs, buf);
    }

    /** csapp 11.8 **/
    /** 疑问：跑adder是能正常工作的，但换成spin就出问题：我想在每一次睡眠之后都给网页发数据(spintf之后printf字符串指针，然后fflush)，但是并没有显示； **/
    sigset_t mask, prev;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, &prev);
    signal(SIGCHLD, sigchild_handler);

    if (Fork() == 0) { /* child */ //line:netp:servedynamic:fork
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1); //line:netp:servedynamic:setenv
        Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */ //line:netp:servedynamic:dup2

        sigprocmask(SIG_SETMASK, &prev, NULL);

        Execve(filename, emptylist, environ); /* Run CGI program */ //line:netp:servedynamic:execve
    }
    sigprocmask(SIG_SETMASK, &prev, NULL);
}
/* $end serve_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */

/* $begin connect_server */
int connect_server(char* host_ip, char* port, char* query_path) {
    static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
    //是在进入这个函数之前连接服务器还是进来之后再连接？answer:进来之后连接
    char buf[MAXLINE];
    int server_fd;

    //连接服务器
    server_fd = open_clientfd(host_ip, port);
    if (server_fd < 0) {
        printf("Fail to connect server!!\n\n");
        return 0;
    }

    //往服务器写http报文
    sprintf(buf, "GET %s HTTP/1.0\r\n", query_path);
    Rio_writen(server_fd, buf, strlen(buf));
    sprintf(buf, "Host: %s\r\n", user_agent_hdr);
    Rio_writen(server_fd, buf, strlen(buf));
    Rio_writen(server_fd, user_agent_hdr, strlen(user_agent_hdr));
    Rio_writen(server_fd,"\r\n",strlen("\r\n"));

    printf("request to server is done.");
    //打印运行消息
    return server_fd;
}
/* $end connect_server */