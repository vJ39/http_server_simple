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

void http (int *sockfd);
void fork_process(int *server_fd);
void sendmes(int *sockfd, char *mes);
void error(char *mes);
void ignore_sigpipe();
int changeroot();
void mime(int *, char *);

int main(){
    int server_fd, pid;
    struct sockaddr_in server_address; // <netinet/in.h>
    socklen_t server_address_len;
    int max_children = 3;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(5000); // <arpa/inet.h>
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // <arpa/inet.h>

    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    server_address_len = sizeof(server_address);

    if(bind(server_fd, (struct sockaddr *)&server_address, server_address_len) == -1) { printf("err: bind()\n"); return -1; }
    if(listen(server_fd, max_children*5) == -1) { printf("err: listen()\n"); return -1; }

    if( changeroot() == -1 ) { return 0; }
    ignore_sigpipe();

    int childid;
    for(childid = 0; childid < max_children; childid++) {
        if((pid = fork()) == 0) {
            fork_process(&server_fd);
        }
    }
    return 0;
}
int changeroot() {
    char docroot[1024] = {"/home/ec2-user/webapp/static"};
    if( chdir(docroot) == -1 ) {
        error("chdir failed\n");
        return -1;
    }
    if( chroot(docroot) == -1 ) {
        int errsv = errno;
        if(errsv == EACCES) { error("1\n"); }
        if(errsv == EFAULT) { error("2\n"); }
        if(errsv == EIO) { error("3\n"); }
        if(errsv == ELOOP) { error("4\n"); }
        if(errsv == ENAMETOOLONG) { error("5\n"); }
        if(errsv == ENOENT) { error("6\n"); }
        if(errsv == ENOMEM) { error("7\n"); }
        if(errsv == ENOTDIR) { error("8\n"); }
        if(errsv == EPERM) { error("9\n"); }
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
        error("sigpipe\n");
    }
}
void fork_process(int *server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    socklen_t client_address_len;

    uid_t uid = 2;
    gid_t gid = 2;

    setuid(uid);
    setgid(gid);

    while(1){
        while((client_fd = accept(*server_fd, (struct sockaddr *)&client_address, &client_address_len)) == -1) { error("accept\n"); sleep(1); };
        http(&client_fd);
        if(client_fd == -1) {
            error("accept failed\n");
        }
    }
    error("end server sock\n");
    shutdown(*server_fd, SHUT_WR);
    close(*server_fd);
}

void http(int *sockfd) {
    char buf[0x10000] = {};
    char meth_name[16];
    char uri_addr[256];
    char http_ver[64];
    int fd;
    read(*sockfd, &buf, sizeof(buf));
    sscanf(buf, "%s %s %s", meth_name, uri_addr, http_ver);
    memset(buf, 0, sizeof(buf));

    if (strcmp(meth_name, "GET") != 0) {
        sendmes(sockfd, "501 Not Implemented\n");
        sendmes(sockfd, "Server: C lang\n");
    }
    else {
        // strncat(docroot, uri_addr, strlen(uri_addr) + strlen(docroot));
        fd = open(uri_addr, O_RDONLY);
        if(fd == -1) {
            error("not found\n");
            sendmes(sockfd, "HTTP/1.0 404 NOT FOUND\n");
            sendmes(sockfd, "Server: C lang\n");
            sendmes(sockfd, "Content-Type: text/html\n\n");
            sendmes(sockfd, "<html><body><h1>404 NOT FOUND</h1></body></html>");
        }
        else {
            read(fd, &buf, sizeof(buf));
            close(fd);
            sendmes(sockfd, "HTTP/1.0 200 OK\n");
            sendmes(sockfd, "Server: C lang\n");
            mime(sockfd, uri_addr);
            sendmes(sockfd, buf);
        }
    }
    if( (shutdown(*sockfd, SHUT_WR)) == -1) { error("shutdown\n"); return; }
    if( (close(*sockfd)) == -1) { error("close\n"); }
}
void sendmes(int *sockfd, char *mes) {
    write(*sockfd, mes, strlen(mes));
}
void error(char *mes) {
    write(1, mes, strlen(mes));
}
void mime(int *sockfd, char *uri) {
    regex_t regst;
    regmatch_t match[1];
    char *reg;
    int flag = 0;
    reg = "\\.jpg$";
    if(!regcomp(&regst, reg, REG_EXTENDED)){
        if(!regexec(&regst, uri, 1, match, 0)) {
            sendmes(sockfd, "Content-Type: image/jpg\n\n");
            flag = 1;
        }
    }
    regfree(&regst);
    reg = "\\.gif$";
    if(!regcomp(&regst, reg, REG_EXTENDED)){
        if(!regexec(&regst, uri, 1, match, 0)) {
            sendmes(sockfd, "Content-Type: image/gif\n\n");
            flag = 1;
        }
    }
    regfree(&regst);
    reg = "\\.png$";
    if(!regcomp(&regst, reg, REG_EXTENDED)){
        if(!regexec(&regst, uri, 1, match, 0)) {
            sendmes(sockfd, "Content-Type: image/png\n\n");
            flag = 1;
        }
    }
    regfree(&regst);
    reg = "\\.js$";
    if(!regcomp(&regst, reg, REG_EXTENDED)){
        if(!regexec(&regst, uri, 1, match, 0)) {
            sendmes(sockfd, "Content-Type: text/plain\n\n");
            flag = 1;
        }
    }
    regfree(&regst);
    reg = "\\.css$";
    if(!regcomp(&regst, reg, REG_EXTENDED)){
        if(!regexec(&regst, uri, 1, match, 0)) {
            sendmes(sockfd, "Content-Type: text/css\n\n");
            flag = 1;
        }
    }
    regfree(&regst);
    reg = "\\.html?$";
    if(!regcomp(&regst, reg, REG_EXTENDED)){
        if(!regexec(&regst, uri, 1, match, 0)) {
            sendmes(sockfd, "Content-Type: text/html\n\n");
            flag = 1;
        }
    }
    regfree(&regst);
    if(!flag) { sendmes(sockfd, "Content-Type: text/plain\n\n"); }
}
