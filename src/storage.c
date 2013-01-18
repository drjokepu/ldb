#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "storage.h"
#include "db.h"
#include "util.h"

static const char *db_storage_file_name = "storage.bin";

bool open_storage(database *restrict db)
{
    char *db_storage_path = create_db_path(db, db_storage_file_name);
    if (access(db_storage_path, R_OK | W_OK) == 0)
    {
        db->storage_file_descriptor = open(db_storage_path, O_RDWR);
    }
    else
    {
        if (errno == ENOENT)
        {
            db->storage_file_descriptor = open(db_storage_path, O_RDWR | O_EXCL | O_CREAT);
            fchmod(db->storage_file_descriptor, S_IRUSR | S_IWUSR);
        }
        else
        {
            perror(program_invocation_short_name);
            free(db_storage_path);
            return false;
        }
    }
    
    free(db_storage_path);
    return true;
}

void close_storage(database *restrict db)
{
    close(db->storage_file_descriptor);
    db->storage_file_descriptor = -1;
}

unsigned long long store_data(const database *restrict db, const void *data, size_t length)
{
    flock(db->storage_file_descriptor, LOCK_EX);
    const unsigned long long offset = lseek(db->storage_file_descriptor, 0, SEEK_END);
    write(db->storage_file_descriptor, data, length);
    return (unsigned long long)offset;
}
