#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <stdbool.h>
#include "typedefs.h"

bool open_storage(database *restrict db);
void close_storage(database *restrict db);

unsigned long long store_data(const database *restrict db, const void *data, size_t length);

#endif /* __STORAGE_H__ */