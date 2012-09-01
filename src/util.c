#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "util.h"

#ifndef _GNU_SOURCE
char *program_invocation_short_name;
#endif /* _GNU_SOURCE */

char *copy_string(const char *restrict str)
{
    const size_t length = strlen(str);
    char *copy = malloc(length + 1);
    strcpy(copy, str);
    return copy;
}

off_t get_file_size(const int file_descriptor)
{
    struct stat file_stat;
    fstat(file_descriptor, &file_stat);
    return file_stat.st_size; 
}

