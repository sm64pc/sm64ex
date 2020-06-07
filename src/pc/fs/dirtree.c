#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../platform.h"
#include "fs.h"
#include "dirtree.h"

static inline uint32_t dirtree_hash(const char *s, size_t len) {
    // djb hash
    uint32_t hash = 5381;
    while (len--) hash = ((hash << 5) + hash) ^ *(s++);
    return hash & (FS_NUMBUCKETS - 1);
}

bool fs_dirtree_init(fs_dirtree_t *tree, const size_t entry_len) {
    memset(tree, 0, sizeof(*tree));

    tree->root = malloc(entry_len);
    if (!tree->root) return false;

    tree->root->name = ""; // root
    tree->root->is_dir = true;
    tree->entry_len = entry_len;

    return true;
}

void fs_dirtree_free(fs_dirtree_t *tree) {
    if (!tree) return;
    if (tree->root) free(tree->root);
    for (int i = 0; i < FS_NUMBUCKETS; ++i) {
        fs_dirtree_entry_t *ent, *next;
        for (ent = tree->buckets[i]; ent; ent = next) {
            next = ent->next_hash;
            free(ent);
        }
    }
}

static inline fs_dirtree_entry_t *dirtree_add_ancestors(fs_dirtree_t *tree, char *name) {
    fs_dirtree_entry_t *ent = tree->root;

    // look for parent directory
    char *last_sep = strrchr(name, '/');
    if (!last_sep) return ent;
    *last_sep = 0;
    ent = fs_dirtree_find(tree, name); 

    if (ent) {
        *last_sep = '/'; // put the separator back
        return ent; // parent directory already in tree
    }

    // add the parent directory
    ent = fs_dirtree_add(tree, name, true);
    *last_sep = '/';

    return ent;
}

fs_dirtree_entry_t *fs_dirtree_add(fs_dirtree_t *tree, char *name, const bool is_dir) {
    fs_dirtree_entry_t *ent = fs_dirtree_find(tree, name);
    if (ent) return ent;

    // add the parent directory into the tree first
    fs_dirtree_entry_t *parent = dirtree_add_ancestors(tree, name);
    if (!parent) return NULL;

    // we'll plaster the name at the end of the allocated chunk, after the actual entry
    const size_t name_len = strlen(name);
    const size_t allocsize = tree->entry_len + name_len + 1;
    ent = calloc(1, allocsize);
    if (!ent) return NULL;

    ent->name = (const char *)ent + tree->entry_len; 
    strcpy((char *)ent->name, name);

    const uint32_t hash = dirtree_hash(name, name_len);
    ent->next_hash = tree->buckets[hash];
    tree->buckets[hash] = ent;
    ent->next_sibling = parent->next_child;
    ent->is_dir = is_dir;
    parent->next_child = ent;

    return ent;
}

fs_dirtree_entry_t *fs_dirtree_find(fs_dirtree_t *tree, const char *name) {
    if (!name) return NULL;
    if (!*name) return tree->root;

    const uint32_t hash = dirtree_hash(name, strlen(name));

    fs_dirtree_entry_t *ent, *prev = NULL;
    for (ent = tree->buckets[hash]; ent; ent = ent->next_hash) {
        if (!strcmp(ent->name, name)) {
            // if this path is not in front of the hash list, move it to the front
            // in case of reccurring searches
            if (prev) {
                prev->next_hash = ent->next_hash;
                ent->next_hash = tree->buckets[hash];
                tree->buckets[hash] = ent;
            }
            return ent;
        }
        prev = ent;
    }

    return NULL;
}

static fs_walk_result_t dirtree_walk_impl(fs_dirtree_entry_t *ent, walk_fn_t walkfn, void *user, const bool recur) {
    fs_walk_result_t res = FS_WALK_SUCCESS;;
    ent = ent->next_child;
    while (ent && (res == FS_WALK_SUCCESS)) {
        if (ent->is_dir) {
            if (recur && ent->next_child)
                res = dirtree_walk_impl(ent, walkfn, user, recur);
        } else if (!walkfn(user, ent->name)) {
            res = FS_WALK_INTERRUPTED;
            break;
        }
        ent = ent->next_sibling;
    }
    return res;
}

fs_walk_result_t fs_dirtree_walk(void *pack, const char *base, walk_fn_t walkfn, void *user, const bool recur) {
    fs_dirtree_t *tree = (fs_dirtree_t *)pack;

    fs_dirtree_entry_t *ent = fs_dirtree_find(tree, base);
    if (!ent) return FS_WALK_NOTFOUND;

    return dirtree_walk_impl(ent, walkfn, user, recur);
}
