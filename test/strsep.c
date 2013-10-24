#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main() {
    char *header, *token;

    header = strdup("UserAgent: Browser/1.0");
    // key
    token = strsep((char **)&header, ":");
    printf("[key] %s\n", token);
    // value
    token = strsep((char **)&header, ":");
    printf("[val] %s\n", token+1);

    free(header);
    return 0;
}
