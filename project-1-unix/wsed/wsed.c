#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strlen, strstr, strcmp
#include <unistd.h> // getopt
#include <getopt.h> // getopt_long


void print_usage() {
    printf("wsed: [option] [val1] [val2] [file]\n");
}

// Substitution
void substitution(char *line, char *val1, char *val2) {

    char *current = line;
    char *found;

    size_t val1_len = strlen(val1);
    size_t val2_len = strlen(val2);

    // strstr returns a pointer to where val1 is found, or NULL if not found
    while ((found = strstr(current, val1)) != NULL) {   
        fwrite(current, 1, found - current, stdout);//print everything before the match
        fwrite(val2, 1, val2_len, stdout); //print val2 instead of val1
        current = found + val1_len; // move current to the part after the match
    }
    printf("%s", current); //print the rest
}


// Translation
void translation(char *line, char *val1, char *val2) {

    for (int i = 0; line[i] != '\0'; i++) {
        int found = 0;
        for (int j = 0; val1[j] != '\0'; j++) {
            if (line[i] == val1[j]) {
                printf("%c", val2[j]);
                found = 1;
                break;
            }
        }

        if (found == 0) {
            printf("%c", line[i]);
        }
    }
}


int main(int argc, char *argv[]) {

    int c;
    char *mode = NULL;
    int help = 0;

    // making --help to be handled the same way as -h
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };


    while ((c = getopt_long(argc, argv, "m:h", long_options, NULL)) != -1) {

        switch (c) {

        case 'm':

            // More than one -m was given
            if (mode != NULL) {
                print_usage();
                return 1;
            }

            mode = optarg;
            break;

        case 'h':
            help = 1;
            break;

        case '?':
            return 1;

        default:
            print_usage();
            return 1;
        }
    }


    // -h or --help
    if (help) {

        if (mode != NULL) {
            print_usage();
            return 1;
        }

        printf("Usage: wsed [-m mode] [initial] [final] [file]\n");
        printf("-m  mode\n");
        printf("\t\"substitution\" or \"translation\". Default: substitution.\n");
        printf("initial & final\n");
        printf("\t in substitution, all occurrences of initial in [file] are replaced with final\n");
        printf("\t in translation, each character in initial is replaced with the the corresponding character from final in [file]\n");

        return 0;
    }

    // substitution by default
    if (mode == NULL) {
        mode = "substitution";
    }

    int remaining = argc - optind;

    if (remaining < 2 || remaining > 3) {
        print_usage();
        return 1;
    }

    char *val1 = argv[optind];
    char *val2 = argv[optind + 1];

    FILE *stream;
    if (remaining == 3) {
        stream = fopen(argv[optind + 2], "r");
        if (stream == NULL) {
            printf("wsed: cannot open file\n");
            return 1;
        }

    } else {
        stream = stdin;
    }

    // Translation - if different lengths:
    if (strcmp(mode, "translation") == 0) {
        if (strlen(val1) != strlen(val2)) {
            printf("wsed: Translation strings not equal length\n");
            if (stream != stdin) {
                fclose(stream);
            }
            return 1;
        }

    } else if (strcmp(mode, "substitution") != 0) {
        printf("wsed: unknown mode detected\n");
        printf("mode must be \"substitution\" or \"translation\"\n");

        if (stream != stdin) {
            fclose(stream);
        }
        return 1;
    }

    char *line = NULL;
    size_t size = 0;
    ssize_t nread;


    while ((nread = getline(&line, &size, stream)) != -1) {
        if (strcmp(mode, "substitution") == 0) {
            substitution(line, val1, val2);
        }else {
            translation(line, val1, val2);
        }
    }


   
    free(line);  // feeling the memory allocated by getline()

    if (stream != stdin) {
        fclose(stream);
    }
    return 0;
}