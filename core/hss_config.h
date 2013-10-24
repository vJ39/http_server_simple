#define HSS_DOCROOT "/home/ec2-user/webapp/static"
#define HSS_LISTEN_PORT 5000
#define HSS_LISTEN_ADDR INADDR_ANY
#define HSS_MAX_CHILDREN 3
#define HSS_PROCESS_UID 2
#define HSS_PROCESS_GID 2

#include <arpa/inet.h>
typedef struct hss_sock {
    int fd;
    socklen_t len;
    struct sockaddr_in ad;
} HSS_SOCK;

typedef struct hss_req {
    char method[16];
    char uri[1024];
    char ver[64];
    int fd;
} HSS_REQ;

typedef struct hss_res {
    char status_code[4];
    char status_message[20];
    char type[0xff];
} HSS_RES;
