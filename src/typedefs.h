#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

typedef enum db_record_type db_record_type;
typedef struct database database;
typedef struct db_map db_map;
typedef struct db_record db_record;
typedef struct db_key_value_pair db_key_value_pair;

typedef enum btree_comparison_result btree_comparison_result;
typedef struct btree btree;
typedef struct btree_callbacks btree_callbacks;
typedef struct btree_map btree_map;
typedef struct btree_node btree_node;

#endif /* __TYPEDEFS_H__ */