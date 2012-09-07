#ifndef __BTREE_H__
#define __BTREE_H__

#include <stdbool.h>
#include <sys/types.h>
#include "typedefs.h"

enum btree_comparison_result
{
    btree_comparison_result_less = -1,
    btree_comparison_result_equals = 0,
    btree_comparison_result_more = 1
};

struct btree_node
{
    unsigned long long parent_node_index;
    unsigned long long left_child_index;
    unsigned long long right_child_index;
    unsigned long long key;
    unsigned long long value;
};

struct btree_map
{
    unsigned long long length;
    unsigned long long capacity;
    btree_node nodes[];
};

struct btree_callbacks
{
    btree_comparison_result (*compare_keys)(unsigned long long key1, unsigned long long key2);
    bool (*key_already_exists)(unsigned long long key, unsigned long long old_value, unsigned long long new_value);
    void (*extend)(btree *restrict tree, unsigned long long new_capacity);
};

struct btree
{
    btree_callbacks callbacks;
    btree_map *map;
};

btree *btree_new();
btree *btree_new_with_capacity(unsigned long long capacity);
void btree_free(btree *tree);
size_t btree_map_size(unsigned long long capacity);

unsigned long long btree_add_node(btree *restrict tree, unsigned long long key, unsigned long long value);
bool btree_get_value(const btree *restrict tree, unsigned long long key, unsigned long long *restrict out_value);
bool btree_has_key(const btree *restrict tree, unsigned long long key);

#endif /* __BTREE_H__ */