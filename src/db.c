#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "db.h"
#include "storage.h"
#include "util.h"

static const char *magic_bytes = "LDB01$";
static const char *db_records_file_name = "records.bin";

static bool db_init(database *restrict db, const char *restrict records_file_name);
static size_t get_db_size(unsigned long long record_count);
static void db_extend(database *restrict db);
static void use_latest_db_version(database *restrict db);
static void db_remap(database *restrict db);
static void load_records_map_details(database *restrict db);

database *db_new(void)
{
    database *db = calloc(1, sizeof(database));
    return db;
}

void db_free(database *db)
{
   if (db->db_path != NULL) free(db->db_path);
   free(db);
}

bool db_open(database* restrict db)
{
    char *db_records_path = create_db_path(db, db_records_file_name);

    const int records_accessibility = access(db_records_path, R_OK | W_OK);
    if (records_accessibility != 0)
    {
        if (errno == ENOENT)
        {
            if (!db_init(db, db_records_file_name))
            {
                return false;
            }
        }
        else
        {
            free(db_records_path);
            perror(program_invocation_short_name);
            return false;
        }
    }

    const int records_file_descriptor = open(db_records_path, O_RDWR | O_CREAT | O_SHLOCK);
    free(db_records_path);
    if (records_file_descriptor < 0)
    {
        perror(program_invocation_short_name);
        return false;
    }

    const off_t records_file_size = get_file_size(records_file_descriptor);
    const void *records_map = mmap(0, records_file_size, PROT_READ | PROT_WRITE, MAP_SHARED, records_file_descriptor, 0);
    
    if (records_map == MAP_FAILED)
    {
        perror(program_invocation_short_name);
        flock(records_file_descriptor, LOCK_UN);
        close(records_file_descriptor);
        return false;
    }

    db->records_file_descriptor = records_file_descriptor;
    db->records_map_size = records_file_size;
    db->records_map = (db_map*)records_map;
    
    load_records_map_details(db);
    open_storage(db);

    flock(records_file_descriptor, LOCK_UN);

    return true;
}

static void load_records_map_details(database *restrict db)
{
    db->last_known_version = db->records_map->version;
}

void db_close(database *restrict db)
{
    munmap(db->records_map, db->records_map_size);
    close(db->records_file_descriptor);
    db->records_map = NULL;
    db->records_file_descriptor = -1;
    close_storage(db);
}

static bool db_init(database *restrict db, const char *restrict records_file_name)
{
    static const unsigned long long default_max_record_count = 512;
    const size_t db_size = get_db_size(default_max_record_count);
    
    char *db_records_path = create_db_path(db, db_records_file_name);

    db_map *map = calloc(1, db_size);
    memcpy(map->magic_bytes, magic_bytes, strlen(magic_bytes));
    map->max_record_count = default_max_record_count;

    const int file_descriptor = open(records_file_name, O_RDWR | O_CREAT | O_EXCL | O_EXLOCK);
    free(db_records_path);
    if (file_descriptor < 0)
    {
        perror(program_invocation_short_name);
        return false;
    }
    fchmod(file_descriptor, S_IRUSR | S_IWUSR);

    write(file_descriptor, map, db_size);
    flock(file_descriptor, LOCK_UN);
    close(file_descriptor);
    return true;
}

static size_t get_db_size(unsigned long long record_count)
{
    return sizeof(db_map) + (record_count * sizeof(db_record));
}

unsigned long long db_add_record(database *restrict db, db_record *record)
{
    flock(db->records_file_descriptor, LOCK_EX);
    use_latest_db_version(db);

    const unsigned long long index = db->records_map->record_count;
    if (index + 1 >= db->records_map->max_record_count)
    {
        db_extend(db);
    }

    db->records_map->record_count++;
    memcpy(&(db->records_map->records[index]), record, sizeof(db_record));
    flock(db->records_file_descriptor, LOCK_UN);

    return index;
}

static void db_extend(database *restrict db)
{
    const unsigned long long old_size = db->records_map_size;
    const unsigned long long new_max_record_count = db->records_map->max_record_count * 2;
    const unsigned long long new_size = get_db_size(new_max_record_count);
    munmap(db->records_map, old_size);
    
    ftruncate(db->records_file_descriptor, new_size);

    db->records_map = mmap(0, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, db->records_file_descriptor, 0);
    db->records_map->max_record_count = new_max_record_count;
    db->records_map->version++;
    db->last_known_version = db->records_map->version;
    db->records_map_size = new_size;
}

static void use_latest_db_version(database *restrict db)
{
    if (db->last_known_version != db->records_map->version)
    {
        db_remap(db);
    }
}

static void db_remap(database *restrict db)
{
    const unsigned long long old_size = db->records_map_size;
    const unsigned long long new_size = get_db_size(db->records_map->max_record_count);
    
    munmap(db->records_map, old_size);
    db->records_map = mmap(0, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, db->records_file_descriptor, 0);
    db->records_map_size = new_size;
    load_records_map_details(db);
}

char *create_db_path(const database *restrict db, const char *restrict filename)
{
    const size_t path_database = strlen(db->db_path);
    char *records_path = malloc(path_database + 32);
    sprintf(records_path, "%s/%s", db->db_path, filename);
    return records_path;
}