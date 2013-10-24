#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hss.h"

/*
 * struct hss_sock *c_ptr = client socket
 * struct hss_res  *r_ptr = response header
 * */
// void response_header(struct hss_sock *c_ptr, struct hss_res *r_ptr) {
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
int main(){
    int i, pid;
    struct hss_sock s_sock;

    hss_process_chroot(DOCROOT);
    hss_process_ignore_sigpipe();
    hss_socket_create(&s_sock, MAX_CHILDREN);

    for(i = 0; i < MAX_CHILDREN; i++) {
        if((pid = fork()) == 0)
            worker(&s_sock);
    }
    return 0;
}

void worker(struct hss_sock *s_ptr) {
    struct hss_sock c_sock, *c_ptr; c_ptr = &c_sock;
    while(1){
        c_ptr->fd = accept(s_ptr->fd, (struct sockaddr *)&c_ptr->ad, &c_ptr->len);
        if(c_ptr->fd == -1) { error("accept"); }
        http();
    }
}

void http() {
    char buf[0xffff] = {0};
    struct hss_req req_h, *req_ptr; req_ptr = &req_h;
    struct hss_res res_h, *res_ptr; res_ptr = &res_h;

    // receive request
    read(c_ptr->fd, &buf, sizeof(buf));
    sscanf(buf, "%s %s %s", req_ptr->method, req_ptr->uri, req_ptr->ver);

    // parse request header
    hss_parse_request(req_ptr, res_ptr);

    // output response header
    response_header(c_ptr, res_ptr);

    // output response body
    if(req_ptr->fd >= 3) {
        char buf[0xffff] = {0};
        int readbytes;
        do {
            readbytes = read(req_ptr->fd, &buf, sizeof(buf));
            write(c_ptr->fd, buf, readbytes);
        } while(readbytes > 0);
        close(req_ptr->fd);
    }
    // HTTP error response
    else {
        if(!strcmp(res_ptr->status_code, "501"))
            resp(c_ptr, "<html><body><h1>501 Not Implemented</h1></body></html>");
        if(!strcmp(res_ptr->status_code, "403"))
            resp(c_ptr, "<html><body><h1>403 Forbidden</h1></body></html>");
        if(!strcmp(res_ptr->status_code, "404"))
            resp(c_ptr, "<html><body><h1>404 Not Found</h1></body></html>");
    }
    hss_socket_close(c_ptr);
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
