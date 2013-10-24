#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(void) {
    char *header;

    header = strdup("Content-Type: text/html\r\n");
    header = strdup("User-Agent: hogehoge/1.0\r\n");
    printf("%s", header);
    
    free(header);
    return 0;
}
