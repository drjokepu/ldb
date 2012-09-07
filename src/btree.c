#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "btree.h"
#include "typedefs.h"

static const unsigned long long default_btree_capacity = 64;

static unsigned long long btree_next_index(const btree *restrict tree);
static unsigned long long btree_add_node_to_parent(btree *restrict tree, unsigned long long key, unsigned long long value, unsigned long long parent);
static unsigned long long btree_add_node_at_index(btree *restrict tree, unsigned long long key, unsigned long long value, unsigned long long parent, unsigned long long index);
static bool btree_needs_extension(const btree *restrict tree);
static void btree_extend(btree *restrict tree);
static bool btree_get_value_at_index(const btree *restrict tree, unsigned long long key, unsigned long long *restrict out_value, unsigned long long index);

btree *btree_new()
{
    return btree_new_with_capacity(default_btree_capacity);
}

btree *btree_new_with_capacity(unsigned long long capacity)
{
    btree *tree = calloc(1, sizeof(btree));
    tree->map = calloc(1, btree_map_size(capacity));
    tree->map->capacity = capacity;
    return tree;
}

void btree_free(btree *tree)
{
    if (tree == NULL) return;
    free(tree->map);
    free(tree);
}

size_t btree_map_size(unsigned long long capacity)
{
    return sizeof(unsigned long long) + sizeof(unsigned long long) + (capacity * sizeof(btree_node));
}

static unsigned long long btree_next_index(const btree *restrict tree)
{
    return tree->map->length + 1;
}

unsigned long long btree_add_node(btree *restrict tree, unsigned long long key, unsigned long long value)
{
    if (tree->map->length > 0)
        return btree_add_node_to_parent(tree, key, value, 0);
    else
        return btree_add_node_at_index(tree, key, value, 0, 0);
}

static unsigned long long btree_add_node_to_parent(btree *restrict tree, unsigned long long key, unsigned long long value, unsigned long long parent)
{
    btree_node *node = tree->map->nodes + parent;
    const btree_comparison_result comparison_result = tree->callbacks.compare_keys(node->key, key);
    switch (comparison_result)
    {
        case btree_comparison_result_less:
            if (node->left_child_index > 0)
                return btree_add_node_to_parent(tree, key, value, node->left_child_index);
            else
                return (node->left_child_index = btree_add_node_at_index(tree, key, value, parent, btree_next_index(tree)));
        case btree_comparison_result_more:
            if (node->right_child_index > 0)
                return btree_add_node_to_parent(tree, key, value, node->right_child_index);
            else
                return (node->right_child_index = btree_add_node_at_index(tree, key, value, parent, btree_next_index(tree)));
        case btree_comparison_result_equals:
            if (tree->callbacks.key_already_exists(key, node->value, value))
                node->value = value;
            return parent;
        default:
            fprintf(stderr, "Unknown btree_comparison_result value: %i\n", comparison_result);
            exit(1);
            return 0;
    }
}

static unsigned long long btree_add_node_at_index(btree *restrict tree, unsigned long long key, unsigned long long value, unsigned long long parent, unsigned long long index)
{
    tree->map->length++;
    if (btree_needs_extension(tree)) btree_extend(tree);
    
    btree_node *node = tree->map->nodes + index;
    node->parent_node_index = parent;
    node->left_child_index = 0;
    node->right_child_index = 0;
    node->key = key;
    node->value = value;
    
    return index;
}

static bool btree_needs_extension(const btree *restrict tree)
{
    return tree->map->length + 1 >= tree->map->capacity;
}

static void btree_extend(btree *restrict tree)
{
    const unsigned long long new_capacity = tree->map->capacity * 2;
    
    if (tree->callbacks.extend != NULL)
        tree->callbacks.extend(tree, new_capacity);
    else
        tree->map = realloc(tree->map, btree_map_size(new_capacity));
    
    tree->map->capacity = new_capacity;
}

bool btree_has_key(const btree *restrict tree, unsigned long long key)
{
    return btree_get_value(tree, key, NULL);
}

bool btree_get_value(const btree *restrict tree, unsigned long long key, unsigned long long *restrict out_value)
{
    return btree_get_value_at_index(tree, key, out_value, 0);
}

static bool btree_get_value_at_index(const btree *restrict tree, unsigned long long key, unsigned long long *restrict out_value, unsigned long long index)
{
    const btree_node *node = tree->map->nodes + index;
    const btree_comparison_result comparison_result = tree->callbacks.compare_keys(node->key, key);
    switch (comparison_result)
    {
        case btree_comparison_result_less:
            if (node->left_child_index > 0)
                return btree_get_value_at_index(tree, key, out_value, node->left_child_index);
            else
                return false;
        case btree_comparison_result_more:
            if (node->right_child_index > 0)
                return btree_get_value_at_index(tree, key, out_value, node->right_child_index);
            else
                return false;
        case btree_comparison_result_equals:
            if (out_value != NULL) *out_value = node->value;
            return true;
        default:
            fprintf(stderr, "Unknown btree_comparison_result value: %i\n", comparison_result);
            exit(1);
            return false;
    }
}