#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
static void usage(const char *a){fprintf(stderr,"Usage: %s <pid>\n",a); exit(1);}
static int isnum(const char*s){for(;*s;s++) if(!isdigit(*s)) return 0; return 1;}
int main(int argc, char **argv) {
    if (argc != 2 || !isnum(argv[1])) usage(argv[0]);

    char path[256], line[256];
    FILE *f;

    snprintf(path, sizeof(path), "/proc/%s/stat", argv[1]);
    f = fopen(path, "r");
    if (!f) { perror("stat"); exit(1); }

    int pid, ppid;
    char comm[256], state;
    unsigned long utime, stime;

    fscanf(f, "%d %255s %c %d", &pid, comm, &state, &ppid);
    for (int i = 0; i < 9; i++) fscanf(f, "%*s");
    fscanf(f, "%lu %lu", &utime, &stime);
    fclose(f);

    long freq = sysconf(_SC_CLK_TCK);
    double cpu = (utime + stime) / (double)freq;


    snprintf(path, sizeof(path), "/proc/%s/cmdline", argv[1]);
    f = fopen(path, "r");
    if (!f) { perror("cmdline"); exit(1); }

    char cmd[256];
    size_t n = fread(cmd, 1, sizeof(cmd) - 1, f);
    fclose(f);

    if (n == 0) strcpy(cmd, "[empty]");
    else {
        for (size_t i = 0; i < n - 1; i++)
            if (cmd[i] == '\0') cmd[i] = ' ';
        cmd[n] = '\0';
    }


    snprintf(path, sizeof(path), "/proc/%s/status", argv[1]);
    f = fopen(path, "r");
    if (!f) { perror("status"); exit(1); }

    char vmrss[64] = "0";
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS:%63s", vmrss);
            break;
        }
    }
    fclose(f);

    printf("PID:%d\n", pid);
    printf("State:%c\n", state);
    printf("PPID:%d\n", ppid);
    printf("Cmd:%s\n", cmd);
    printf("CPU:%ld %.3f\n", freq, cpu);
    printf("VmRSS:%s\n", vmrss);

    return 0;
}
