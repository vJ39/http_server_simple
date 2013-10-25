#include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/jail.h>
#include <unistd.h>
#include <stdio.h>

#define MAXPATHLEN 1024
#define MAXHOSTNAMELEN 256

int main(){
    struct jail str_j;
    int jid;

    char path[MAXPATHLEN] = {"/jail"};
    char hostname[MAXHOSTNAMELEN] = {"jail.server"};

    str_j.version = 0;
    str_j.path = path;
    str_j.hostname = hostname;
    if( inet_aton("192.168.30.250", str_j.ip4) == 0 ) {
        printf("error\n");
        return -1;
    };

    printf("%s\n", inet_ntoa(*str_j.ip4));

    jid = jail(&str_j);
    sleep(10);
    return 0;
}
