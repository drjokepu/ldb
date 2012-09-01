#ifndef __DB_H__
#define __DB_H__

#include <stdbool.h>
#include <sys/types.h>
#include "typedefs.h"

struct database
{
    char *db_path;
    int records_file_descriptor;
    int storage_file_descriptor;
    size_t records_map_size;
    unsigned long long last_known_version;
    db_map *records_map;
};

enum db_record_type
{
    db_record_type_tuple_head = 0,
    db_record_type_typle_tail = 1
};

struct db_record
{
    unsigned long long record_id;
    db_record_type record_type;
    bool inactive;
    union
    {
        struct
        {
            unsigned long long element_count;
            unsigned long long tail_index;
        } tuple_head;
        struct
        {
            unsigned long long name_storage_index;
            unsigned long long storage_index;
            unsigned long long tail_index;
        } tuple_tail;
    } data;
};

struct db_map
{
    char magic_bytes[6];
    unsigned long long version;
    unsigned long long record_count;
    unsigned long long max_record_count;

    db_record records[];
};

database *db_new(void);
void db_free(database *db);

bool db_open(database *restrict db);
void db_close(database *restrict db);

unsigned long long db_add_record(database *restrict db, db_record *record);

char *create_db_path(const database *restrict db, const char *restrict filename);

#endif /* __DB_H__ */
