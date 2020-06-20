#ifndef _SM64_DIRTREE_H_
#define _SM64_DIRTREE_H_

#include <stdlib.h>
#include <stdbool.h>

#include "fs.h"

#define FS_NUMBUCKETS 64

typedef struct fs_dirtree_entry_s {
    const char *name;
    bool is_dir;
    struct fs_dirtree_entry_s *next_hash, *next_child, *next_sibling;
} fs_dirtree_entry_t;

typedef struct {
    fs_dirtree_entry_t *root;
    fs_dirtree_entry_t *buckets[FS_NUMBUCKETS];
    size_t entry_len;
} fs_dirtree_t;

bool fs_dirtree_init(fs_dirtree_t *tree, const size_t entry_len);
void fs_dirtree_free(fs_dirtree_t *tree);

fs_dirtree_entry_t *fs_dirtree_add(fs_dirtree_t *tree, char *name, const bool is_dir);
fs_dirtree_entry_t *fs_dirtree_find(fs_dirtree_t *tree, const char *name);

// the first arg is void* so this could be used in walk() methods of various packtypes
fs_walk_result_t fs_dirtree_walk(void *tree, const char *base, walk_fn_t walkfn, void *user, const bool recur);

#endif // _SM64_DIRTREE_H_
