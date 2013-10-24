#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(void) {
    char *header;

    header = strdup("Content-Type: text/html");

    char *sep = ": ", *v, *brkt;

    v = strtok_r(header, sep, &brkt);
    printf("[key] %s\n", v);
    v = strtok_r(NULL, sep, &brkt);
    printf("[val] %s\n", v);

    free(header);
    return 0;
}
