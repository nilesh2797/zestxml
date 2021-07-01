#include "stats.h"

using namespace std;

int parseProcLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getRAMUsedByProc()
{ 	// function to get the RAM used by the process ON UBUNTU, value is in MB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseProcLine(line);
            break;
        }
    }
    fclose(file);
    return result / 1024.0;
}