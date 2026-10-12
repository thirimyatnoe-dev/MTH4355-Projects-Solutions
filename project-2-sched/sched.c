/* References for the functions used so far:
 * getline: https://man7.org/linux/man-pages/man3/getline.3.html
 * strsep: https://man7.org/linux/man-pages/man3/strsep.3.html
 * strtol and ERANGE: https://man7.org/linux/man-pages/man3/strtol.3.html
 * realloc: https://man7.org/linux/man-pages/man3/realloc.3.html
 * isspace: https://man7.org/linux/man-pages/man3/isspace.3.html
 * getopt: https://man7.org/linux/man-pages/man3/getopt.3.html
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int arrival;
    int run_length;
    int io_time;
    int io_length;
    int cpu_done;
    int blocked_until;
    int first_start;
    int finish_time;
} Process;

void print_header(size_t process_count) {
    printf("t  |");
    for (size_t i = 0; i < process_count; i++) {
        printf(" Process %c |", 'A' + (int)i);
    }
    printf("\n");

}

size_t update_arrivals_and_io(Process *processes, size_t process_count, int time) {
    size_t finished_count = 0;
    for (size_t i = 0; i < process_count; i++) {
        if (processes[i].arrival == time) {
            // zero CPU time needed
            if (processes[i].run_length == 0) {
                processes[i].first_start = time;
                processes[i].finish_time = time;
                finished_count++;
                continue;
            }

            // I/O before any CPU work
            if (processes[i].io_time == 0 && processes[i].io_length > 0) {
                processes[i].blocked_until = time + processes[i].io_length;
                continue;
            }
        }
    }

    for (size_t i = 0; i < process_count; i++) {
        if (processes[i].blocked_until > 0 && processes[i].blocked_until == time) {
            processes[i].blocked_until = 0;
        }
    }

    return finished_count;
}

int is_ready(Process *processes, size_t i, int time) {
    return processes[i].arrival <= time && // the process has arrived 
           processes[i].blocked_until <= time && // not waiting for I/O
           processes[i].cpu_done < processes[i].run_length; // still has CPU time left
}

void print_states(Process *processes, size_t process_count, int time, int running) {
    printf("%-3d|", time);
    for (size_t i = 0; i < process_count; i++) {
        const char *state;
        if (processes[i].arrival > time) {
            state = "";
        } else if (processes[i].cpu_done == processes[i].run_length) {
            state = "Dead";
        } else if (processes[i].blocked_until > time) { 
            state = "Blocked";
        } else if ((int)i == running) {
            state = "Running";
        } else {
            state = "Runnable";
        }
        printf(" %-10s|", state);
    }
    printf("\n");

}

// return 1 if the process finishes or blocks, otherwise 0
int run_cpu_unit(Process *processes, int running, int time) {
    if (processes[running].first_start == -1) {
        processes[running].first_start = time;
    }
    processes[running].cpu_done++;

    if (processes[running].cpu_done == processes[running].run_length) {
        processes[running].finish_time = time + 1;
        return 1;
    }
    // Check for i/o
    if (processes[running].io_length > 0 && // do we have i/o
        processes[running].cpu_done == processes[running].io_time) { // have we completed the amt of cpu work at which it should request i/o
        processes[running].blocked_until = time + 1 + processes[running].io_length;
        return 1;
    }
    return 0;
}

void print_averages(Process *processes, size_t process_count) {
    double turnaround_total = 0;
    double response_total = 0;

    for (size_t i = 0; i < process_count; i++) {
        turnaround_total += processes[i].finish_time - processes[i].arrival;
        response_total += processes[i].first_start - processes[i].arrival;
    }

    double average_turnaround = 0;
    double average_response = 0;
    if (process_count > 0) {
        average_turnaround = turnaround_total / process_count;
        average_response = response_total / process_count;
    }

    printf("Average Turn-around Time: %.3f\n", average_turnaround);
    printf("Average Response Time: %.3f\n", average_response);
}

int run_fifo(Process *processes, size_t process_count) {
    int time = 0;
    int running = -1;
    size_t finished_count = 0;

    print_header(process_count);
    while (finished_count < process_count) {
        finished_count += update_arrivals_and_io(processes, process_count, time);

        // FIFO: earliest original arrival
        if (running == -1) {
            for (size_t i = 0; i < process_count; i++) {
                if (is_ready(processes, i, time)) { 
                    if (running == -1 || processes[i].arrival < processes[running].arrival) {
                        running = (int)i;
                    }
                }
            }
        }

        print_states(processes, process_count, time, running);
        if (running != -1 && run_cpu_unit(processes, running, time) == 1) {
            if (processes[running].cpu_done == processes[running].run_length) {
                finished_count++;
            }
            running = -1;
        }
        if (finished_count < process_count) {
            time++;
        }
    }
    print_averages(processes, process_count);
    return 0;
}

int run_sjf(Process *processes, size_t process_count) {
    int time = 0;
    int running = -1;
    size_t finished_count = 0;

    print_header(process_count);
    while (finished_count < process_count) {
        finished_count += update_arrivals_and_io(processes, process_count, time);

        // SJF: shortest total CPU time
        if (running == -1) {
            for (size_t i = 0; i < process_count; i++) {
                if (is_ready(processes, i, time)) {
                    if (running == -1 || processes[i].run_length < processes[running].run_length) {
                        running = (int)i;
                    }
                }
            }
        }

        print_states(processes, process_count, time, running);
        if (running != -1 && run_cpu_unit(processes, running, time) == 1) {
            if (processes[running].cpu_done == processes[running].run_length) {
                finished_count++;
            }
            running = -1;
        }
        if (finished_count < process_count) {
            time++;
        }
    }
    print_averages(processes, process_count);
    return 0;
}

int run_stcf(Process *processes, size_t process_count) {
    int time = 0;
    int running = -1;
    size_t finished_count = 0;

    print_header(process_count);
    while (finished_count < process_count) {
        finished_count += update_arrivals_and_io(processes, process_count, time);

        // STCF: reconsider every time unit using remaining CPU work
        running = -1;
        for (size_t i = 0; i < process_count; i++) {
            if (is_ready(processes, i, time)) {
                int remaining = processes[i].run_length - processes[i].cpu_done;
                if (running == -1 ||
                    remaining < processes[running].run_length - processes[running].cpu_done) { // to check if the current process i is better choice than the shortest one we have so far(running)
                    running = (int)i;
                }
            }
        }

        print_states(processes, process_count, time, running);
        if (running != -1 && run_cpu_unit(processes, running, time) == 1) {
            if (processes[running].cpu_done == processes[running].run_length) {
                finished_count++;
            }
            running = -1;
        }
        if (finished_count < process_count) {
            time++;
        }
    }
    print_averages(processes, process_count);
    return 0;
}

int run_rr(Process *processes, size_t process_count) {
    int time = 0;
    int running = -1;
    size_t finished_count = 0;
    size_t next_process = 0;

    print_header(process_count);
    while (finished_count < process_count) {
        finished_count += update_arrivals_and_io(processes, process_count, time);

        // RR: one CPU unit, then look for the next ready process
        running = -1;
        for (size_t checked = 0; checked < process_count; checked++) {
            size_t i = (next_process + checked) % process_count; // wrapping around
            if (is_ready(processes, i, time)) {
                running = (int)i;
                next_process = (i + 1) % process_count;
                break;
            }
        }

        print_states(processes, process_count, time, running);
        if (running != -1 && run_cpu_unit(processes, running, time) == 1) {
            if (processes[running].cpu_done == processes[running].run_length) {
                finished_count++;
            }
            running = -1;
        }
        if (finished_count < process_count) {
            time++;
        }
    }
    print_averages(processes, process_count);
    return 0;
}

int main(int argc, char *argv[]) {
    const char *mode = "FIFO"; // default to FIFO
    int mode_seen = 0;
    int option;
    opterr = 0; 

    while ((option = getopt(argc, argv, "m:")) != -1) {
        // unknown option, missing mode, or repeated -m
        if (option != 'm' || mode_seen) {
            printf("sched -m [mode] < [process file]\n");
            return 1;
        }
        mode = optarg;
        mode_seen = 1; // to  rmb that -m was used (to avoid overwriting modes)
    }

    // extra arguments or unknown policy
    if (optind != argc ||
        (strcmp(mode, "FIFO") != 0 && strcmp(mode, "SJF") != 0 &&
         strcmp(mode, "STCF") != 0 && strcmp(mode, "RR") != 0)) {
        printf("sched -m [mode] < [process file]\n");
        return 1;
    }

    char *line = NULL;
    size_t capacity = 0;
    Process *processes = NULL;
    size_t process_count = 0;

    while (getline(&line, &capacity, stdin) != -1) {
        char *remaining = line;
        char *field;
        int values[4];
        int count = 0;

        field = strsep(&remaining, ",");

        while (field != NULL) {
            // too many fields
            if (count == 4) {
                printf("Invalid process detected\n");
                free(processes);
                free(line);
                return 1;
            }

            char *end; // where strtol stopped reading
            errno = 0; // reset before checking for a number that is too large
            long value = strtol(field, &end, 10); // reads a decimal number (base 10)

            // no number found
            if (end == field) {
                printf("Invalid process detected\n");
                free(processes);
                free(line);
                return 1;
            }

            // skip whitespace
            while (isspace((unsigned char)*end)) {
                end++;
            }

            // extra text, number too large, or negative number
            if (*end != '\0' || errno == ERANGE ||
                value < 0 || value > INT_MAX) {
                printf("Invalid process detected\n");
                free(processes);
                free(line);
                return 1;
            }

            values[count] = (int)value;
            count++;
            field = strsep(&remaining, ",");
        }

        // too few fields
        if (count != 4) {
            printf("Invalid process detected\n");
            free(processes);
            free(line);
            return 1;
        }

        Process p;
        p.arrival = values[0];
        p.run_length = values[1];
        p.io_time = values[2];
        p.io_length = values[3];
        p.cpu_done = 0;
        p.blocked_until = 0;
        p.first_start = -1;
        p.finish_time = -1;

        // make room for one more process
        Process *new_processes = realloc(processes, (process_count + 1) * sizeof(Process));
        if (new_processes == NULL) {
            fprintf(stderr, "Unable to allocate memory\n");
            free(processes);
            free(line);
            return 1;
        }

        processes = new_processes;
        processes[process_count] = p;
        process_count++;
    }

    if (ferror(stdin)) {
        fprintf(stderr, "Unable to read input\n");
        free(processes);
        free(line);
        return 1;
    }

    int result;
    if (strcmp(mode, "FIFO") == 0) {
        result = run_fifo(processes, process_count);
    } else if (strcmp(mode, "SJF") == 0) {
        result = run_sjf(processes, process_count);
    } else if (strcmp(mode, "STCF") == 0) {
        result = run_stcf(processes, process_count);
    } else {
        result = run_rr(processes, process_count);
    }

    free(processes);
    free(line);
    return result;
}
