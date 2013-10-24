#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(void) {
    char *header;

    header = strdup("GET / HTTP/1.0\nHost: 192.168.130.128:5000\nUser-Agent: Mozilla/5.0 (Windows NT 5.1; rv:24.0) Gecko/20100101 Firefox/24.0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\nAccept-Language: ja,en-us;q=0.7,en;q=0.3\nAccept-Encoding: gzip, deflate\nConnection: keep-alive\nCache-Control: max-age=0\n\n");

    char *lin = "\n", *v, *pt, *tk;

    for(v = strtok_r(header, lin, &pt); v; v = strtok_r(NULL, lin, &pt)) {
        // key
        tk = strsep((char **)&v, ": ");
        printf("[line] %s = ", tk);
        // value
        tk = strsep((char **)&v, "\n");
        printf("%s\n", tk);
    }

    free(header);
    return 0;
}
