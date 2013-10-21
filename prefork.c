#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void http (int sockfd);
void sendmes(int sockfd, char *mes);
void fork_process(int *server_fd);
int main(){
    int server_fd, pid;
    struct sockaddr_in server_address; // <netinet/in.h>
    socklen_t server_address_len;
//    char server_sockopt = 1;
    int max_children = 3;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(5000); // <arpa/inet.h>
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // <arpa/inet.h>

    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    server_address_len = sizeof(server_address);

//    setsockopt(server_fd, SOL_SOCKET, NULL, &server_sockopt, sizeof(server_sockopt));
    if(bind(server_fd, (struct sockaddr *)&server_address, server_address_len) == -1) { printf("err: bind()\n"); return -1; }
    if(listen(server_fd, max_children*5) == -1) { printf("err: listen()\n"); return -1; }

    int childid;
    for(childid = 0; childid < max_children; childid++) {
        if((pid = fork()) == 0) {
            fork_process(&server_fd);
        }
    }
    return 0;
}

void fork_process(int *server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    socklen_t client_address_len;
//    char client_sockopt = 1;

    while(1){
        client_fd = accept(*server_fd, (struct sockaddr *)&client_address, &client_address_len);
//        setsockopt(client_fd, SOL_SOCKET, NULL, &client_sockopt, sizeof(client_sockopt));
        http(client_fd);
    }
    shutdown(*server_fd, SHUT_WR);
    close(*server_fd);

}

void http(int sockfd) {
    char buf[0x10000] = {};
    char meth_name[16];
    char uri_addr[256];
    char http_ver[64];
    char docroot[1024] = {"/home/webapp/public_html"};
    int fd;
    read(sockfd, &buf, sizeof(buf));
    sscanf(buf, "%s %s %s", meth_name, uri_addr, http_ver);
    memset(buf, 0, sizeof(buf));

    if (strcmp(meth_name, "GET") != 0) {
        sendmes(sockfd, "501 Not Implemented\n");
        sendmes(sockfd, "Server: C lang\n");
    }
    else {
        strncat(docroot, uri_addr, strlen(uri_addr) + strlen(docroot));
        fd = open(docroot, O_RDONLY);
        if(fd == -1) {
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
            sendmes(sockfd, "Content-Type: text/html\n\n");
            sendmes(sockfd, buf);
        }
    }
    shutdown(sockfd, SHUT_WR);
    read(sockfd, &buf, sizeof(buf));
    close(sockfd);
}
void sendmes(int sockfd, char *mes) {
    write(sockfd, mes, strlen(mes));
}
