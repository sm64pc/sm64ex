// logfile.c - handles opening and closing of the log file

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define LOGFILE_NAME "errorlog.txt"

static bool firstOpen = true;
static bool active = false;
static FILE* logfile = NULL;

bool logfile_open(FILE** f) {
    if (active) {
        *f = logfile;
        return true;
    }

    printf("Initiating logfile to '%s'\n", LOGFILE_NAME);

    logfile = fopen(fs_get_write_path(LOGFILE_NAME), "a");
    if (logfile == NULL) { return false; }
    *f = logfile;

    if (firstOpen) {
        fprintf(logfile, "--- new run ---\n");
        firstOpen = false;
    }

    active = true;
    return logfile;
}

void logfile_close(void) {
    if (!active) { return; }
    fflush(logfile);
    fclose(logfile);
    active = false;
}