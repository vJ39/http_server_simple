#include <stdio.h>
#include <string.h>

// difinition
struct hss_cgi_kv {
    char key[0xff];
    char val[0xfff];
};
void setval(char *, struct hss_cgi_kv *);

void hss_cgi_setenv(struct hss_cgi_kv *env, char *str) {
    setval(str, env);
}
void setval(char *v, struct hss_cgi_kv *e) {
    strncat(v, e->key, strlen(e->key));
    strncat(v, "=", (size_t)1);
    strncat(v, e->val, strlen(v)+strlen(e->val));
}
