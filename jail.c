#include <sys/types.h>
#include <sys/jail.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXPATHLEN 1024
#define MAXHOSTNAMELEN 256

int main(){
    struct jail str_j;
    struct in_addr *addr;
    int jid;

    char path[MAXPATHLEN] = {"/usr/home/yotsutake/webapp"};
    char hostname[MAXHOSTNAMELEN] = {"jail.server"};

    str_j.version = 0;
    str_j.path = path;
    str_j.hostname = hostname;
    inet_aton("192.168.1.199", addr);
    str_j.ip4 = addr;

    jid = jail(&str_j);
    return 0;
}
