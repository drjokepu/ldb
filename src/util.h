#ifndef __UTIL_H__
#define __UTIL_H__

#include <sys/types.h>

char *copy_string(const char *restrict str);
off_t get_file_size(const int file_descriptor);

#ifndef _GNU_SOURCE
extern char *program_invocation_short_name;
#endif /* _GNU_SOURCE */

#endif /* __UTIL_H__ */
