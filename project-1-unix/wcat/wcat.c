#include <stdio.h>

int main(int argc, char *argv[]) {

    if (argc == 1) {
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("wcat: cannot open file\n");
            return 1;
        }
        char buffer[2000];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) { //read line from the file, fp and then put it into buffer?
            printf("%s", buffer);
        }
        fclose(fp);
    }
    return 0;
}