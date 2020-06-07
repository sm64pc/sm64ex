#ifndef _SM64_FS_H_
#define _SM64_FS_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "../platform.h"

// FS_BASEDIR is usually defined in the build script
#ifndef FS_BASEDIR
# define FS_BASEDIR "res"
#endif

#ifndef FS_BASEPACK_PREFIX
# define FS_BASEPACK_PREFIX "base"
#endif

#define FS_TEXTUREDIR "gfx"
#define FS_SOUNDDIR "sound"

extern char fs_gamedir[];
extern char fs_writepath[];

// receives the full path
// should return `true` if traversal should continue
// first arg is user data
typedef bool (*walk_fn_t)(void *, const char *);

typedef enum {
    FS_WALK_SUCCESS = 0,
    FS_WALK_INTERRUPTED = 1,
    FS_WALK_NOTFOUND = 2,
    FS_WALK_ERROR = 4,
} fs_walk_result_t;

// opaque searchpath directory type
typedef struct fs_dir_s fs_dir_t;

// virtual file handle
typedef struct fs_file_s {
    void *handle;     // opaque packtype-defined data
    fs_dir_t *parent; // directory containing this file
} fs_file_t;

// list of paths; returned by fs_enumerate()
typedef struct {
    char **paths;
    int numpaths;
    int listcap;
} fs_pathlist_t;

typedef struct {
    const char *extension; // file extensions of this pack type

    void *(*mount)(const char *rpath); // open and initialize pack at real path `rpath`
    void (*unmount)(void *pack);       // free pack

    // walks the specified directory inside this pack, calling walkfn for each file
    // returns FS_WALK_SUCCESS if the directory was successfully opened and walk() didn't ever return false
    // returns FS_WALK_INTERRUPTED if the traversal started but walk() returned false at some point
    // if recur is true, will recurse into subfolders
    fs_walk_result_t (*walk)(void *pack, const char *base, walk_fn_t walkfn, void *user, const bool recur);

    bool (*is_file)(void *pack, const char *path); // returns true if `path` exists in this pack and is a file
    bool (*is_dir)(void *pack, const char *path);  // returns true if `path` exists in this pack and is a directory

    // file I/O functions; paths are virtual
    fs_file_t *(*open)(void *pack, const char *path); // opens a virtual file contained in this pack for reading, returns NULL in case of error
    int64_t (*read)(void *pack, fs_file_t *file, void *buf, const uint64_t size); // returns -1 in case of error
    bool (*seek)(void *pack, fs_file_t *file, const int64_t ofs); // returns true if seek succeeded
    int64_t (*tell)(void *pack, fs_file_t *file); // returns -1 in case of error, current virtual file position otherwise
    int64_t (*size)(void *pack, fs_file_t *file); // returns -1 in case of error, size of the (uncompressed) file otherwise
    bool (*eof)(void *pack, fs_file_t *file);     // returns true if there's nothing more to read
    void (*close)(void *pack, fs_file_t *file);   // closes a virtual file previously opened with ->open()
} fs_packtype_t;

// takes the supplied NULL-terminated list of read-only directories and mounts all the packs in them,
// then mounts the directories themselves, then mounts all the packs in `gamedir`, then mounts `gamedir` itself,
// then does the same with `userdir`
// initializes the `fs_gamedir` and `fs_userdir` variables
bool fs_init(const char **rodirs, const char *gamedir, const char *userdir);

// mounts the pack at physical path `realpath` to the root of the filesystem
// packs mounted later take priority over packs mounted earlier
bool fs_mount(const char *realpath);

// removes the pack at physical path from the virtual filesystem
bool fs_unmount(const char *realpath);

/* generalized filesystem functions that call matching packtype functions for each pack in the searchpath */

// FIXME: this can walk in unorthodox patterns, since it goes through mountpoints linearly
fs_walk_result_t fs_walk(const char *base, walk_fn_t walkfn, void *user, const bool recur);

// returns a list of files in the `base` directory
fs_pathlist_t fs_enumerate(const char *base, const bool recur);
// call this on a list returned by fs_enumerate() to free it
void fs_pathlist_free(fs_pathlist_t *pathlist);

bool fs_is_file(const char *fname);
bool fs_is_dir(const char *fname);

fs_file_t *fs_open(const char *vpath);
void fs_close(fs_file_t *file);
int64_t fs_read(fs_file_t *file, void *buf, const uint64_t size);
const char *fs_readline(fs_file_t *file, char *dst, const uint64_t size);
bool fs_seek(fs_file_t *file, const int64_t ofs);
int64_t fs_tell(fs_file_t *file);
int64_t fs_size(fs_file_t *file);
bool fs_eof(fs_file_t *file);

void *fs_load_file(const char *vpath, uint64_t *outsize);
const char *fs_readline(fs_file_t *file, char *dst, uint64_t size);

// tries to find the first file with the filename that starts with `prefix`
// puts full filename into `outname` and returns it or returns NULL if nothing matches
const char *fs_match(char *outname, const size_t outlen, const char *prefix);

// takes a virtual path and prepends the write path to it
const char *fs_get_write_path(const char *vpath);

// expands special chars in paths and changes backslashes to forward slashes
const char *fs_convert_path(char *buf, const size_t bufsiz, const char *path);

/* these operate on the real filesystem and are used by fs_packtype_dir */

bool fs_sys_walk(const char *base, walk_fn_t walk, void *user, const bool recur);
fs_pathlist_t fs_sys_enumerate(const char *base, const bool recur);
bool fs_sys_file_exists(const char *name);
bool fs_sys_dir_exists(const char *name);
bool fs_sys_mkdir(const char *name); // creates with 0777 by default

#endif // _SM64_FS_H_
