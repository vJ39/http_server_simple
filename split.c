#include <stdio.h>
#include <string.h>
/*
#define MAX_LEN 100
int main() {
    int i, len;
    char str[] = "Cookie: hogehoge\r\nUser-Agent: Mozilla/5.0";
    char *words[MAX_LEN], *cp;
    const char *delim = "\n";

    cp = str;
    for(len = 0; len < MAX_LEN; len++) {
        if((words[len] = strtok(cp, delim)) == NULL)
            break;
        cp = 0;
    }
    for(i = 0; i < len; i++) {
        puts(words[i]);
    }
    return 0;
}
*/
int main() {
    char test[80];
    char *line = "\r\n";
    char *sep = ":";
    char *word, *phrase, *brkt, *brkb;

    strcpy(test, "Content-Type: hogehoge\nUser-Agent: fugafuga\r\n");

    for (word = strtok_r(test, line, &brkt); word; word = strtok_r(NULL, line, &brkt)) {
        phrase = strtok_r(word, sep, &brkb);
        phrase = strtok_r(NULL, sep, &brkb);
        printf("So far we're at %s:%s\n", word, phrase);
    }
    return 0;
}
