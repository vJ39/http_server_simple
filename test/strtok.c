#include <string.h>
#include <stdio.h>
int main(void) {
    char *str = "Content-Type: text/html\r\nUser-Agent: Browser/1.0\r\n";
    char *line = "\r\n", *delim = ": ";
    char *ptr_line, *ptr_delim;
    for(str = strtok_r(str, line, &ptr_line); str; str = strtok_r(NULL, line, &ptr_line)) {
        str = strtok_r(str, delim, &ptr_delim);
        str = strtok_r(NULL, delim, &ptr_delim);
        printf("%s\n", str);
    }
    return 0;
}
