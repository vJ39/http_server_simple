#include <stdio.h>
#include "hss_cgi.h"
int main(void) {
    char s[1024] = {0};
    struct hss_cgi_kv e[] = {
        (struct hss_cgi_kv) {"HTTP_HOST", "webapp.ith.jp"},
        (struct hss_cgi_kv) {"HTTP_USER_AGENT", "Mozilla/5.0 (Windows NT 5.1; rv:24.0) Gecko/20100101 Firefox/24.0"},
        (struct hss_cgi_kv) {"HTTP_ACCEPT", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"}
    };
    int i;
    for(i = 0; i < (int)(sizeof(e) / sizeof(e[0])); i++) {
        hss_cgi_setenv(&e[i], s);
        printf("%s\n", s);
        memset(s, 0, sizeof(s));
    }
    return 0;
}
