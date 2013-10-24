#include "hss_socket.h"

HSS_SOCK s_sock; // global server sock

int hss_socket_open() {
    HSS_SOCK *s_ptr = &s_sock;

    // create server sock
    s_ptr->ad.sin_family = AF_INET;
    s_ptr->ad.sin_port = htons(HSS_LISTEN_PORT); // <arpa/inet.h>
    s_ptr->ad.sin_addr.s_addr = htonl(HSS_LISTEN_ADDR); // <arpa/inet.h>
    s_ptr->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    s_ptr->len = sizeof(s_ptr->ad);

    if( bind(s_ptr->fd, (struct sockaddr *)&s_ptr->ad, s_ptr->len) == -1)
        return -1;
    if( listen(s_ptr->fd, HSS_MAX_CHILDREN * 5) == -1)
        return -1;
    return 0;
}

int hss_socket_close(HSS_SOCK *c_ptr) {
    if( shutdown(c_ptr->fd, SHUT_WR) == -1 )
        return -1;
    if( close(c_ptr->fd) == -1 )
        return -1;
    return 0;
}

void hss_response(HSS_SOCK *c_ptr, char *mes) {
    write(c_ptr->fd, mes, strlen(mes));
    write(c_ptr->fd, "\n", 1);
}

void hss_socket_listener(HSS_SOCK *s_ptr) {
    c_ptr->fd = accept(s_ptr->fd, (struct sockaddr *)&c_ptr->ad, &c_ptr->len);
    if(c_ptr->fd == -1) { error("accept"); }
    http(c_ptr);
}

