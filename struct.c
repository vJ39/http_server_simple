#include <stdio.h>
#include <string.h>

struct kv {
    char key[0xff];
    char val[0xfff];
};

void setval(char *, struct kv *);

int main(int argc, char *argv[]) {
    int p = 0;
    char v[0xff+0xfff][0xff];
    struct kv e[10], *e_ptr;
    e[0] = (struct kv) {"REMOTE_ADDR", "192.168.0.10"};
    e_ptr = e;
    setval(v[p], e_ptr);
//    e = (struct kv){"QUERY_STRING", "foo=1&bar=2"};
    printf("%s\n", v[p]);
    return 0;
}
void setval(char *v, struct kv *e){
    strncat(v, e->key, strlen(e->key));
    strncat(v, "=", (size_t)1);
    strncat(v, e->val, strlen(v)+strlen(e->val));
}
