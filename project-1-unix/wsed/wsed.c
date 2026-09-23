#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[]){
    int opt;
    int mode_count = 0;
    char *mode = "substitution";
    
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, "m:h", long_options, NULL)) != -1) {
        if (opt == 'm') {
            mode_count++;

            if (mode_count > 1) {
                printf("wsed: [option] [val1] [val2] [file]\n");
                return 1;
            }

            mode = optarg;
        }  
        else if (opt == 'h') {
            printf("Usage: wsed [-m mode] [initial] [final] [file]\n");
            printf("-m  mode\n");
            printf("\t\"substitution\" or \"translation\". Default: substitution.\n");
            printf("initial & final\n");
            printf("\t in substitution, all occurrences of initial in [file] are replaced with final\n");
            printf("\t in translation, each character in initial is replaced with the the corresponding character from final in [file]\n");
            return 0;
        }
        else {
            printf("wsed: [option] [val1] [val2] [file]\n");
            return 1;
        }
        }
        if (argc - optind != 3) {
            printf("wsed: [option] [val1] [val2] [file]\n");
            return 1;
        }
        char *val1 = argv[optind];
        char *val2 = argv[optind + 1];
        char *filename = argv[optind + 2];

        FILE *fp = fopen(filename, "r");

        if (fp == NULL) {
            printf("wsed: cannot open file\n");
            return 1;
        }

        //Substitution
        if (strcmp(mode, "substitution") == 0) {
            char *line = NULL;
            size_t len = 0;

            while (getline(&line, &len, fp) != -1) {
                char *current = line;
                char *match;

                while ((match = strstr(current, val1)) != NULL) {
                    printf("%.*s", (int)(match - current), current);
                    printf("%s", val2);
                    current = match + strlen(val1);
                }
                printf("%s", current);
            }
            fclose(fp);
        }

        //Translation
        else if (strcmp(mode, "translation") == 0) {
        
            if (strlen(val1) != strlen(val2)) {
                printf("wsed: Translation strings not equal length\n");
                fclose(fp);
                return 1;
            }
            int c;

            while ((c = fgetc(fp)) != EOF) { //reads one char at a time, EOF = end of file
                for (int i = 0; i < strlen(val1); i++) {
                    if (c == val1[i]) {
                        c = val2[i];
                        break;  //one translation max for one char 
                    }
                }
                printf("%c", c);
            }
            fclose(fp);
        }
        else {
            //printf("wsed: [option] [val1] [val2] [file]\n");
            printf("wsed: unknown mode detected\n");
            printf("mode must be \"substitution\" or \"translation\"\n");
            return 1;
        }
      
    return 0;
}