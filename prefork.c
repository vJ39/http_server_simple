#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <regex.h>
#include <sys/stat.h>

#define MAX_CHILDREN 3

struct hss_sock {
    int fd;
    socklen_t len;
    struct sockaddr_in ad; // netinet/in.h
};
struct hss_req {
    char method[16];
    char uri[1024];
    char ver[64];
    int fd;
};
struct hss_res {
    char status_code[4];
    char status_message[20];
    char type[0xff];
};

void response_header(struct hss_sock *, struct hss_res *);
void http (struct hss_sock *);
void fork_process(struct hss_sock *);
void res(struct hss_sock *, char *);
void error(char *);
void ignore_sigpipe(void);
int changeroot(void);
void setmimetype(struct hss_req *, struct hss_res *);
void parse_request(struct hss_req *, struct hss_res *);

/*
 * struct hss_sock *c_ptr = client socket
 * struct hss_res  *r_ptr = response header
 * */
void response_header(struct hss_sock *c_ptr, struct hss_res *r_ptr) {
    // HTTP version
    write(c_ptr->fd, "HTTP/1.0 ", 9);
    // status code and message
    write(c_ptr->fd, r_ptr->status_code, 3);
    write(c_ptr->fd, " ", 1);
    write(c_ptr->fd, r_ptr->status_message, strlen(r_ptr->status_message));
    write(c_ptr->fd, "\n", 1);
    // server name
    write(c_ptr->fd, "Server: C lang\n", 15);
    // content type
    write(c_ptr->fd, "Content-Type: ", 14);
    write(c_ptr->fd, r_ptr->type, strlen(r_ptr->type));
    write(c_ptr->fd, "\n", 1);
    // end header
    write(c_ptr->fd, "\n", 1);
}
void res(struct hss_sock *c_ptr, char *mes) {
    write(c_ptr->fd, mes, strlen(mes));
    write(c_ptr->fd, "\n", 1);
}

int main(){
    int pid, ret;
    struct hss_sock s_sock, *s_ptr; s_ptr = &s_sock;

    ret = changeroot();
    if(ret == -1) {
        return -1;
    }
    setuid( (uid_t)2 );
    setgid( (gid_t)2 );
    ignore_sigpipe();

    // create server sock
    s_ptr->ad.sin_family = AF_INET;
    s_ptr->ad.sin_port = htons(5000); // <arpa/inet.h>
    s_ptr->ad.sin_addr.s_addr = htonl(INADDR_ANY); // <arpa/inet.h>
    s_ptr->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    s_ptr->len = sizeof(s_ptr->ad);

    ret = bind(s_ptr->fd, (struct sockaddr *)&s_ptr->ad, s_ptr->len);
    if(ret == -1) {
        error("bind()"); return -1;
    }
    ret = listen(s_ptr->fd, MAX_CHILDREN * 5);
    if(ret == -1) {
        error("listen"); return -1;
    }
    int i;
    for(i = 0; i < MAX_CHILDREN; i++) {
        if((pid = fork()) == 0) {
            fork_process(s_ptr);
        }
    }
    return 0;
}
int changeroot() {
    char docroot[1024] = {"/home/ec2-user/webapp/static"};
    if( chdir(docroot) == -1 ) {
        error("chdir failed");
        return -1;
    }
    if( chroot(docroot) == -1 ) {
        int errsv = errno;
        if(errsv == EACCES) { error("1"); }
        if(errsv == EFAULT) { error("2"); }
        if(errsv == EIO) { error("3"); }
        if(errsv == ELOOP) { error("4"); }
        if(errsv == ENAMETOOLONG) { error("5"); }
        if(errsv == ENOENT) { error("6"); }
        if(errsv == ENOMEM) { error("7"); }
        if(errsv == ENOTDIR) { error("8"); }
        if(errsv == EPERM) { error("9"); }
        return -1;
    }
    return 0;
}
void ignore_sigpipe() {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    if(sigaction(SIGPIPE, &act, 0) == -1) {
        error("sigpipe");
    }
}
void fork_process(struct hss_sock *s_ptr) {
    struct hss_sock c_sock, *c_ptr; c_ptr = &c_sock;
    while(1){
        c_ptr->fd = accept(s_ptr->fd, (struct sockaddr *)&c_ptr->ad, &c_ptr->len);
        if(c_ptr->fd == -1) { error("accept"); }
        http(c_ptr);
    }
}

void parse_request(struct hss_req *req_ptr, struct hss_res *res_ptr) {
    // initialize response header struct
    *res_ptr = (struct hss_res) {"", "", ""};

    // 501 Not Implemented
    if( strcmp(req_ptr->method, "GET") != 0 ) {
        *res_ptr = (struct hss_res) {
            "501",
            "Not Implemented",
            "text/html"
        };
        req_ptr->fd = -1;
        return;
    }

    struct stat st, *st_ptr; st_ptr = &st;
    if( lstat(req_ptr->uri, st_ptr) == -1 ) {
        int errsv = errno;
        if(errsv == ENOENT) {
            *res_ptr = (struct hss_res) {
                "404",
                "Not Found",
                "text/html"
            };
        }
        else {
            *res_ptr = (struct hss_res) {
                "403",
                "Forbidden",
                "text/html"
            };
        }
        req_ptr->fd = -1;
        return;
    }
    // is a directory
    if( S_ISDIR(st_ptr->st_mode) ) {
        strncat(req_ptr->uri, "/index.html", strlen(req_ptr->uri) + 11);
        if( (req_ptr->fd = open(req_ptr->uri, O_RDONLY)) == -1) {
            *res_ptr = (struct hss_res) {
                "403",
                "Forbidden",
                "text/html"
            };
            req_ptr->fd = -1;
            return;
        }
        // 200 OK
        *res_ptr = (struct hss_res) {
            "200",
            "OK",
            "text/html"
        };
        return;
    }
    else {
        // open a regular file
        if( (req_ptr->fd = open(req_ptr->uri, O_RDONLY)) == -1) {
            *res_ptr = (struct hss_res) {
                "403",
                "Forbidden",
                "text/html"
            };
            req_ptr->fd = -1;
            return;
        }
        // 200 OK
        strncpy(res_ptr->status_code, "200", 3);
        strncpy(res_ptr->status_message, "OK", 2);
        setmimetype(req_ptr, res_ptr);
        return;
    }
}

void http(struct hss_sock *c_ptr) {
    char buf[0xffff] = {0};
    int ret;
    struct hss_req req_h, *req_ptr; req_ptr = &req_h;
    struct hss_res res_h, *res_ptr; res_ptr = &res_h;

    // receive request
    read(c_ptr->fd, &buf, sizeof(buf));
    sscanf(buf, "%s %s %s", req_ptr->method, req_ptr->uri, req_ptr->ver);

    // parse request header
    parse_request(req_ptr, res_ptr);

    // output response header
    response_header(c_ptr, res_ptr);

    // output response body
    if(req_ptr->fd >= 3) {
        char buf[0xffff] = {0};
        do {
            ret = read(req_ptr->fd, &buf, sizeof(buf));
            write(c_ptr->fd, buf, ret);
        } while(ret > 0);
        close(req_ptr->fd);
    }
    // HTTP error response
    else {
        if(!strcmp(res_ptr->status_code, "501"))
            res(c_ptr, "<html><body><h1>501 Not Implemented</h1></body></html>");
        if(!strcmp(res_ptr->status_code, "403"))
            res(c_ptr, "<html><body><h1>403 Forbidden</h1></body></html>");
        if(!strcmp(res_ptr->status_code, "404"))
            res(c_ptr, "<html><body><h1>404 Not Found</h1></body></html>");
    }
    ret = shutdown(c_ptr->fd, SHUT_WR);
    if(ret == -1) {
        error("shutdown"); return;
    }
    ret = close(c_ptr->fd);
    if(ret == -1) {
        error("close"); return;
    }
}
void error(char *mes) {
    write(2, mes, strlen(mes));
}
void setmimetype(struct hss_req *req_ptr, struct hss_res *res_ptr) {
    struct mimetypes {
        char regex[0xff];
        char type[0xfff];
    };
    struct mimetypes m_type[] = {
        {"\\.jpg$",  "image/jpg"},
        {"\\.gif$",  "image/gif"},
        {"\\.png$",  "image/png"},
        {"\\.html$", "text/html"},
        {"\\.htm$",  "text/html"},
        {"\\.css$",  "text/css"}
    };

    regex_t regst;
    regmatch_t match[1];
    int i, flag = 0;

    for(i = 0; i < (sizeof(m_type) / sizeof(m_type[0])); i++) {
        if(!regcomp(&regst, m_type[i].regex, REG_EXTENDED)){
            if(!regexec(&regst, req_ptr->uri, 1, match, 0)) {
                strncpy(res_ptr->type, m_type[i].type, strlen(m_type[i].type));
                flag = 1;
            }
        }
        regfree(&regst);
    }
    if(!flag) {
        strncpy(res_ptr->type, "text/plain", 10);
    }
}
