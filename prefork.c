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

#define SERVER_NAME "C lang"
#define MAX_CHILDREN 3

void http (struct hss_sock *);
void fork_process(struct hss_sock *);
void res(struct hss_sock *, char *, size_t);
void error(char *);
void ignore_sigpipe(void);
int changeroot(void);
void mime(struct hss_sock *);

struct hss_sock {
    int fd;
    socklen_t len;
    struct sockaddr_in ad; // netinet/in.h
};

int main(){
    int pid, ret;
    struct hss_sock s_sock, *s__ptr;
    s_ptr = &s_sock;

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
        errmes("bind()"); return -1;
    }
    ret = listen(s_ptr->fd, MAX_CHILDREN * 5);
    if(ret == -1) {
        errmes("listen"); return -1;
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
    char docroot[1024] = {"/Users/yotsutake/webapp/http_server_simple"};
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

void setenvp(int *sock_fd, struct sockaddr *address) {
    char buf[1024];
    memset(buf, 0, sizeof(fields));
    strncpy(envp[*sock_fd], "REMOTE_ADDR=", strlen("REMOTE_ADDR="));
    strncat(envp[*sock_fd], address.sin_addr.s_addr, strlen(envp[*sock_fd]) + strlen(address.sin_addr.s_addr));
    envp[*sock_fd]
}

void http(struct hss_sock *c_ptr) {
    int ret;
    char buf[0x10000] = {};
    struct hss_req {
        char method[16];
        char uri[1024];
        char ver[64];
        int fd;
    } r, *r_ptr; r_ptr = &r;

    read(c_ptr->fd, &buf, sizeof(buf));
    sscanf(buf, "%s %s %s", r_ptr->method, r_ptr->uri, r_ptr->ver);

    if( strcmp(r_ptr->method, "GET") != 0 ) {
        res(c_ptr, "501 Not Implemented", NULL);
        res(c_ptr, SERVER_NAME, NULL);
    } else {
        r_ptr->fd = open(c_ptr->ad, O_RDONLY);
        if(r_ptr->fd == -1) {
            error("NOT FOUND");
            res(c_ptr, "HTTP/1.0 404 NOT FOUND", NULL);
            res(c_ptr, SERVER_NAME, NULL);
            res(c_ptr, "Content-Type: text/html\n", NULL);
            res(c_ptr, "<html><body><h1>404 NOT FOUND</h1></body></html>", NULL);
        } else {
            res(c_ptr, "HTTP/1.0 200 OK", NULL);
            res(c_ptr, SERVER_NAME, NULL);
            mime(c_ptr);
            do {
                ret = read(r_ptr->fd, &buf, sizeof(buf));
                res(c_ptr, buf, ret);
            } while(ret > 0);
            close(r_ptr->fd);
        }
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
void res(struct hss_sock *c_ptr, char *mes, size_t len) {
    if(len == NULL) {
        write(c_ptr->fd, mes, sizeof(mes));
    } else {
        write(c_ptr->fd, mes, len);
    }
}
void error(char *mes) {
    write(2, mes, strlen(mes));
}
void mime(struct hss_sock *c_ptr) {
    struct mimetypes {
        char regex[0xff];
        char type[0xfff];
    };
    struct mimetypes m_type[] = {
        {"jpg", "iamge/jpg"},
        {"gif", "image/gif"},
        {"png", "image/png"},
        {"html", "text/html"},
        {"\\/", "text/html"},
        {"js", "text/plain"},
        {"css", "text/css"}
    };

    regex_t regst;
    regmatch_t match[1];
    int flag = 0, i;

    for(i = 0; i < sizeof(m_type); i++) {
        if(!regcomp(&regst, m_type[i].regex, REG_EXTENDED)){
            if(!regexec(&regst, c_ptr->uri, 1, match, 0)) {
                res(c_ptr->fd, m_type[i].type, NULL);
                res(c_ptr->fd, "\n", NULL);
                flag = 1;
                break;
            }
        }
        regfree(&regst);
    }
    /*
    reg = "\\.cgi?$";
    if(!regcomp(&regst, reg, REG_EXTENDED)){
        if(!regexec(&regst, uri, 1, match, 0)) {
            flag = 1;
            char *envp[];
            execve(uri, NULL, );
        }
    }
    regfree(&regst);
    */
    if(!flag) {
        res(c_tr->fd, "Content-Type: text/plin\n", NULL);
    }
}
