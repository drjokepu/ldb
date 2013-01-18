#include <string.h>
#include "db.h"
#include "util.h"

static int dbmain(int argc, char *argv[]);

int main(int argc, char *argv[])
{
#ifndef _GNU_SOURCE
    program_invocation_short_name = argv[0];
#endif /* _GNU_SOURCE */

    return dbmain(argc, argv);
}

static int dbmain(int argc, char *argv[])
{
    database *db = db_new();
    db->db_path = strdup("."); 
    db_open(db);

    db_close(db);
    db_free(db);

    return 0;
}
