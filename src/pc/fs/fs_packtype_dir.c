#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "macros.h"
#include "../platform.h"
#include "fs.h"

static void *pack_dir_mount(const char *realpath) {
    if (!fs_sys_dir_exists(realpath))
        return NULL;
    // the pack is actually just the real folder path
    void *pack = (void *)sys_strdup(realpath);
    return pack;
}

static void pack_dir_unmount(void *pack) {
    free(pack);
}

struct walkdata_s {
    size_t baselen;
    walk_fn_t userwalk;
    void *userdata;
};

// wrap the actual user walk function to return virtual paths instead of real paths
static bool packdir_walkfn(void *userdata, const char *path) {
    struct walkdata_s *walk = (struct walkdata_s *)userdata;
    return walk->userwalk(walk->userdata, path + walk->baselen);
}

static fs_walk_result_t pack_dir_walk(void *pack, const char *base, walk_fn_t walkfn, void *user, const bool recur) {
    char path[SYS_MAX_PATH];
    snprintf(path, SYS_MAX_PATH, "%s/%s", (const char *)pack, base);

    if (!fs_sys_dir_exists(path))
        return FS_WALK_NOTFOUND;

    struct walkdata_s walkdata = { strlen((const char *)pack) + 1, walkfn, user };
    return fs_sys_walk(path, packdir_walkfn, &walkdata, recur);
}

static bool pack_dir_is_file(void *pack, const char *fname) {
    char path[SYS_MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", (const char *)pack, fname);
    return fs_sys_dir_exists(path);
}

static bool pack_dir_is_dir(void *pack, const char *fname) {
    char path[SYS_MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", (const char *)pack, fname);
    return fs_sys_file_exists(path);
}

static fs_file_t *pack_dir_open(void *pack, const char *vpath) {
    char path[SYS_MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", (const char *)pack, vpath);

    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fs_file_t *fsfile = malloc(sizeof(fs_file_t));
    if (!fsfile) { fclose(f); return NULL; }

    fsfile->parent = NULL;
    fsfile->handle = f;

    return fsfile;
}

static void pack_dir_close(UNUSED void *pack, fs_file_t *file) {
    fclose((FILE *)file->handle);
    free(file);
}

static int64_t pack_dir_read(UNUSED void *pack, fs_file_t *file, void *buf, const uint64_t size) {
    return fread(buf, 1, size, (FILE *)file->handle);
}

static bool pack_dir_seek(UNUSED void *pack, fs_file_t *file, const int64_t ofs) {
    return fseek((FILE *)file->handle, ofs, SEEK_SET) == 0;
}

static int64_t pack_dir_tell(UNUSED void *pack, fs_file_t *file) {
    return ftell((FILE *)file->handle);
}

static int64_t pack_dir_size(UNUSED void *pack, fs_file_t *file) {
    int64_t oldofs = ftell((FILE *)file->handle);
    fseek((FILE *)file->handle, 0, SEEK_END);
    int64_t size = ftell((FILE *)file->handle);
    fseek((FILE *)file->handle, oldofs, SEEK_SET);
    return size;
}

static bool pack_dir_eof(UNUSED void *pack, fs_file_t *file) {
    return feof((FILE *)file->handle);
}

fs_packtype_t fs_packtype_dir = {
    "",
    pack_dir_mount,
    pack_dir_unmount,
    pack_dir_walk,
    pack_dir_is_file,
    pack_dir_is_dir,
    pack_dir_open,
    pack_dir_read,
    pack_dir_seek,
    pack_dir_tell,
    pack_dir_size,
    pack_dir_eof,
    pack_dir_close,
};
