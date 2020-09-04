#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <tinfl.h>

#include "macros.h"
#include "../platform.h"
#include "fs.h"
#include "dirtree.h"

#define ZIP_BUFSIZE 16384
#define ZIP_EOCD_BUFSIZE 65578

#define ZIP_LFH_SIG 0x04034b50
#define ZIP_CDH_SIG  0x02014b50
#define ZIP_EOCD_SIG 0x06054b50

typedef struct {
    fs_dirtree_t tree;    // this should always be first, so this could be used as a dirtree root
    const char *realpath; // physical path to the zip file
    FILE *zipf;           // open zip file handle, if any
} zip_pack_t;

typedef struct {
    fs_dirtree_entry_t tree; // this should always be first, so this could be used as a dirtree entry
    uint64_t ofs;            // offset to compressed data in zip
    uint16_t bits;           // general purpose zip flags
    uint16_t comptype;       // compression method
    uint32_t crc;            // CRC-32
    uint64_t comp_size;      // size of compressed data in zip
    uint64_t uncomp_size;    // size of decompressed data
    uint16_t attr_int;       // internal attributes
    uint32_t attr_ext;       // external attributes
    bool     ofs_fixed;      // if true, `ofs` points to the file data, otherwise to LFH
} zip_entry_t;

typedef struct {
    zip_entry_t *entry;  // the dirtree entry of this file
    uint32_t comp_pos;   // read position in compressed data
    uint32_t uncomp_pos; // read position in uncompressed data
    uint8_t *buffer;     // decompression buffer (if compressed)
    z_stream zstream;    // tinfl zlib stream
    FILE *fstream;       // duplicate of zipf of the parent zip file
} zip_file_t;

static int64_t zip_find_eocd(FILE *f, int64_t *outlen) {
    // the EOCD is somewhere in the last 65557 bytes of the file
    // get the total file size
    fseek(f, 0, SEEK_END);
    const int64_t fsize = ftell(f);
    if (fsize <= 16) return -1; // probably not a zip

    const int64_t rx = (fsize < ZIP_EOCD_BUFSIZE ? fsize : ZIP_EOCD_BUFSIZE);
    uint8_t *buf = malloc(rx);
    if (!buf) return -1;

    // read that entire chunk and search for EOCD backwards from the end
    fseek(f, fsize - rx, SEEK_SET);
    if (fread(buf, rx, 1, f)) {
        for (int64_t i = rx - 8; i >= 0; --i) {
            if ((buf[i + 0] == 0x50) && (buf[i + 1] == 0x4B) &&
                (buf[i + 2] == 0x05) && (buf[i + 3] == 0x06)) {
                // gotem
                free(buf);
                if (outlen) *outlen = fsize;
                return fsize - rx + i;
            }
        }
    }

    free(buf);
    return -1;
}

static bool zip_parse_eocd(FILE *f, uint64_t *cdir_ofs, uint64_t *data_ofs, uint64_t *count) {
    int64_t fsize = 0;

    // EOCD record struct
    struct eocd_s {
        uint32_t sig;
        uint16_t this_disk;
        uint16_t cdir_disk;
        uint16_t disk_entry_count;
        uint16_t total_entry_count;
        uint32_t cdir_size;
        uint32_t cdir_ofs;
        uint16_t comment_len;
        // zip comment follows
    } __attribute__((__packed__));
    struct eocd_s eocd;

    // find the EOCD and seek to it
    int64_t pos = zip_find_eocd(f, &fsize);
    if (pos < 0) return false;
    fseek(f, pos, SEEK_SET);

    // read it
    if (!fread(&eocd, sizeof(eocd), 1, f)) return false;

    // double check the sig
    if (LE_TO_HOST32(eocd.sig) != ZIP_EOCD_SIG) return false;

    // disks should all be 0
    if (eocd.this_disk || eocd.cdir_disk) return false;

    // total entry count should be the same as disk entry count
    if (eocd.disk_entry_count != eocd.total_entry_count) return false;

    *count = LE_TO_HOST16(eocd.total_entry_count);
    *cdir_ofs = LE_TO_HOST32(eocd.cdir_ofs);
    eocd.cdir_size = LE_TO_HOST32(eocd.cdir_size);

    // end of central dir can't be before central dir
    if ((uint64_t)pos < *cdir_ofs + eocd.cdir_size) return false;

    *data_ofs = (uint64_t)(pos - (*cdir_ofs + eocd.cdir_size));
    *cdir_ofs += *data_ofs;

    // make sure end of comment matches end of file
    eocd.comment_len = LE_TO_HOST16(eocd.comment_len);
    return ((pos + 22 + eocd.comment_len) == fsize);
}

static bool zip_fixup_offset(zip_file_t *zipfile) {
    // LFH record struct
    struct lfh_s {
        uint32_t sig;
        uint16_t version_required;
        uint16_t bits;
        uint16_t comptype;
        uint16_t mod_time;
        uint16_t mod_date;
        uint32_t crc;
        uint32_t comp_size;
        uint32_t uncomp_size;
        uint16_t fname_len;
        uint16_t extra_len;
        // file name, extra field and data follow
    } __attribute__((__packed__));

    struct lfh_s lfh;

    zip_entry_t *ent = zipfile->entry;

    fseek(zipfile->fstream, ent->ofs, SEEK_SET);
    if (!fread(&lfh, sizeof(lfh), 1, zipfile->fstream)) return false;

    // we only need these two
    lfh.fname_len = LE_TO_HOST16(lfh.fname_len);
    lfh.extra_len = LE_TO_HOST16(lfh.extra_len);

    // ofs will now point to actual data
    ent->ofs += sizeof(lfh) + lfh.fname_len + lfh.extra_len;
    ent->ofs_fixed = true; // only need to do this once

    return true;
}

static zip_entry_t *zip_load_entry(FILE *f, fs_dirtree_t *tree, const uint64_t data_ofs) {
    // CDH record struct
    struct cdh_s {
        uint32_t sig;
        uint16_t version_used;
        uint16_t version_required;
        uint16_t bits;
        uint16_t comptype;
        uint16_t mod_time;
        uint16_t mod_date;
        uint32_t crc;
        uint32_t comp_size;
        uint32_t uncomp_size;
        uint16_t fname_len;
        uint16_t extra_len;
        uint16_t comment_len;
        uint16_t start_disk;
        uint16_t attr_int;
        uint32_t attr_ext;
        uint32_t lfh_ofs;
        // file name, extra field and comment follow
    } __attribute__((__packed__));

    struct cdh_s cdh;
    zip_entry_t zipent;

    memset(&zipent, 0, sizeof(zipent));

    if (!fread(&cdh, sizeof(cdh), 1, f)) return NULL;

    // check cdir entry header signature
    if (LE_TO_HOST32(cdh.sig) != ZIP_CDH_SIG) return NULL;

    // byteswap and copy some important fields
    zipent.bits = LE_TO_HOST16(cdh.bits);
    zipent.comptype = LE_TO_HOST16(cdh.comptype);
    zipent.crc = LE_TO_HOST32(cdh.crc);
    zipent.comp_size = LE_TO_HOST32(cdh.comp_size);
    zipent.uncomp_size = LE_TO_HOST32(cdh.uncomp_size);
    zipent.ofs = LE_TO_HOST32(cdh.lfh_ofs);
    zipent.attr_int = LE_TO_HOST16(cdh.attr_int);
    zipent.attr_ext = LE_TO_HOST32(cdh.attr_ext);
    cdh.fname_len = LE_TO_HOST16(cdh.fname_len);
    cdh.comment_len = LE_TO_HOST16(cdh.comment_len);
    cdh.extra_len = LE_TO_HOST16(cdh.extra_len);

    // read the name
    char *name = calloc(1, cdh.fname_len + 1);
    if (!name) return NULL;
    if (!fread(name, cdh.fname_len, 1, f)) { free(name); return NULL; }

    // this is a directory if the name ends in a path separator
    bool is_dir = false;
    if (name[cdh.fname_len - 1] == '/') {
        is_dir = true;
        name[cdh.fname_len - 1] = 0;
    }
    name[cdh.fname_len] = 0;

    // add to directory tree
    zip_entry_t *retent = (zip_entry_t *)fs_dirtree_add(tree, name, is_dir);
    free(name);
    if (!retent) return NULL;

    // copy the data we read into the new entry
    zipent.tree = retent->tree;
    memcpy(retent, &zipent, sizeof(zipent));

    // this points to the LFH now; will be fixed up on file open
    // while the CDH includes an "extra field length" field, it's usually different
    retent->ofs += data_ofs;

    // skip to the next CDH
    fseek(f, cdh.extra_len + cdh.comment_len, SEEK_CUR);

    return retent;
}

static inline bool zip_load_entries(FILE *f, fs_dirtree_t *tree, const uint64_t cdir_ofs, const uint64_t data_ofs, const uint64_t count) {
    fseek(f, cdir_ofs, SEEK_SET);
    for (uint64_t i = 0; i < count; ++i) {
        if (!zip_load_entry(f, tree, data_ofs))
            return false;
    }
    return true;
}

static inline bool is_zip(FILE *f) {
    uint32_t sig = 0;
    if (fread(&sig, sizeof(sig), 1, f)) {
        // the first LFH might be at the start of the zip
        if (LE_TO_HOST32(sig) == ZIP_LFH_SIG)
            return true;
        // no signature, might still be a zip because fuck you
        // the only way now is to try and find the end of central directory
        return zip_find_eocd(f, NULL) >= 0;
    }
    return false;
}

static void *pack_zip_mount(const char *realpath) {
    uint64_t cdir_ofs, data_ofs, count;
    zip_pack_t *pack = NULL;
    FILE *f = NULL;

    f = fopen(realpath, "rb");
    if (!f) goto _fail;

    if (!is_zip(f)) goto _fail;

    pack = calloc(1, sizeof(zip_pack_t));
    if (!pack) goto _fail;

    if (!zip_parse_eocd(f, &cdir_ofs, &data_ofs, &count))
        goto _fail;

    if (!fs_dirtree_init(&pack->tree, sizeof(zip_entry_t)))
        goto _fail;

    if (!zip_load_entries(f, &pack->tree, cdir_ofs, data_ofs, count))
        goto _fail;

    pack->realpath = sys_strdup(realpath);
    pack->zipf = f;

    return pack;

_fail:
    if (f) fclose(f);
    if (pack) free(pack);
    return NULL;
}

static void pack_zip_unmount(void *pack) {
    zip_pack_t *zip = (zip_pack_t *)pack;
    fs_dirtree_free(&zip->tree);
    if (zip->realpath) free((void *)zip->realpath);
    if (zip->zipf) fclose(zip->zipf);
    free(zip);
}

static bool pack_zip_is_file(void *pack, const char *fname) {
    zip_entry_t *ent = (zip_entry_t *)fs_dirtree_find((fs_dirtree_t *)pack, fname);
    return ent && !ent->tree.is_dir;
}

static bool pack_zip_is_dir(void *pack, const char *fname) {
    zip_entry_t *ent = (zip_entry_t *)fs_dirtree_find((fs_dirtree_t *)pack, fname);
    return ent && ent->tree.is_dir;
}

static inline void pack_zip_close_zipfile(zip_file_t *zipfile) {
    if (zipfile->buffer) {
        inflateEnd(&zipfile->zstream);
        free(zipfile->buffer);
    }
    if (zipfile->fstream) fclose(zipfile->fstream);
    free(zipfile);
}

static fs_file_t *pack_zip_open(void *pack, const char *vpath) {
    fs_file_t *fsfile = NULL;
    zip_file_t *zipfile = NULL;
    zip_pack_t *zip = (zip_pack_t *)pack;
    zip_entry_t *ent = (zip_entry_t *)fs_dirtree_find((fs_dirtree_t *)zip, vpath);
    if (!ent || ent->tree.is_dir) goto _fail; // we're expecting a fucking file here

    zipfile = calloc(1, sizeof(zip_file_t));
    if (!zipfile) goto _fail;
    zipfile->entry = ent;

    // obtain an additional file descriptor
    // fdopen(dup(fileno())) is not very portable and might not create separate state
    zipfile->fstream = fopen(zip->realpath, "rb");
    if (!zipfile->fstream) goto _fail;

    // make ent->ofs point to the actual file data if it doesn't already
    if (!ent->ofs_fixed)
        if (!zip_fixup_offset(zipfile))
            goto _fail; // this shouldn't generally happen but oh well

    // if there's compression, assume it's zlib
    if (ent->comptype != 0) {
        zipfile->buffer = malloc(ZIP_BUFSIZE);
        if (!zipfile->buffer)
            goto _fail;
        if (inflateInit2(&zipfile->zstream, -MAX_WBITS) != Z_OK)
            goto _fail;
    }

    fsfile = malloc(sizeof(fs_file_t));
    if (!fsfile) goto _fail;
    fsfile->handle = zipfile;
    fsfile->parent = NULL;

    // point to the start of the file data
    fseek(zipfile->fstream, ent->ofs, SEEK_SET);

    return fsfile;

_fail:
    if (zipfile) pack_zip_close_zipfile(zipfile);
    if (fsfile) free(fsfile);
    return NULL;
}

static void pack_zip_close(UNUSED void *pack, fs_file_t *file) {
    if (!file) return;

    zip_file_t *zipfile = (zip_file_t *)file->handle;
    if (zipfile) pack_zip_close_zipfile(zipfile);

    free(file);
}

static int64_t pack_zip_read(UNUSED void *pack, fs_file_t *file, void *buf, const uint64_t size) {
    zip_file_t *zipfile = (zip_file_t *)file->handle;
    zip_entry_t *ent = zipfile->entry;

    int64_t avail = ent->uncomp_size - zipfile->uncomp_pos;
    int64_t max_read = ((int64_t)size > avail) ? avail : (int64_t)size;
    int64_t rx = 0;
    int err = 0;

    if (max_read == 0) return 0;

    if (ent->comptype == 0) {
        // no compression, just read
        rx = fread(buf, 1, size, zipfile->fstream);
    } else {
        zipfile->zstream.next_out = buf;
        zipfile->zstream.avail_out = (unsigned int)max_read;
        while (rx < max_read) {
            const uint32_t before = (uint32_t)zipfile->zstream.total_out;
            // check if we ran out of compressed bytes and read more if we did
            if (zipfile->zstream.avail_in == 0) {
                int32_t comp_rx = ent->comp_size - zipfile->comp_pos;
                if (comp_rx > 0) {
                    if (comp_rx > ZIP_BUFSIZE) comp_rx = ZIP_BUFSIZE;
                    comp_rx = fread(zipfile->buffer, 1, comp_rx, zipfile->fstream);
                    if (comp_rx == 0) break;
                    zipfile->comp_pos += (uint32_t)comp_rx;
                    zipfile->zstream.next_in = zipfile->buffer;
                    zipfile->zstream.avail_in = (unsigned int)comp_rx;
                }
            }
            // inflate
            err = inflate(&zipfile->zstream, Z_SYNC_FLUSH);
            rx += zipfile->zstream.total_out - before;
            if (err != Z_OK) break;
        }
    }

    zipfile->uncomp_pos += rx;
    return rx;
}

static bool pack_zip_seek(UNUSED void *pack, fs_file_t *file, const int64_t ofs) {
    zip_file_t *zipfile = (zip_file_t *)file->handle;
    zip_entry_t *ent = zipfile->entry;
    uint8_t buf[512];

    if (ofs > (int64_t)ent->uncomp_size) return false;

    if (ent->comptype == 0) {
        if (fseek(zipfile->fstream, ofs + ent->ofs, SEEK_SET) == 0)
            zipfile->uncomp_pos = ofs;
    } else {
        // if seeking backwards, gotta redecode the stream from the start until that point
        // so we make a copy of the zstream and clear it with a new one
        if (ofs < zipfile->uncomp_pos) {
            z_stream zstream;
            memset(&zstream, 0, sizeof(zstream));
            if (inflateInit2(&zstream, -MAX_WBITS) != Z_OK)
                return false;
            // reset the underlying file handle back to the start
            if (fseek(zipfile->fstream, ent->ofs, SEEK_SET) != 0)
                return false;
            // free and replace the old one
            inflateEnd(&zipfile->zstream);
            memcpy(&zipfile->zstream, &zstream, sizeof(zstream));
            zipfile->uncomp_pos = zipfile->comp_pos = 0;
        }
        // continue decoding the stream until we hit the new offset
        while (zipfile->uncomp_pos != ofs) {
            uint32_t max_read = (uint32_t)(ofs - zipfile->uncomp_pos);
            if (max_read > sizeof(buf)) max_read = sizeof(buf);
            if (pack_zip_read(pack, file, buf, max_read) != max_read)
                return false;
        }
    }

    return true;
}

static int64_t pack_zip_tell(UNUSED void *pack, fs_file_t *file) {
    return ((zip_file_t *)file->handle)->uncomp_pos;
}

static int64_t pack_zip_size(UNUSED void *pack, fs_file_t *file) {
    zip_file_t *zipfile = (zip_file_t *)file->handle;
    return zipfile->entry->uncomp_size;
}

static bool pack_zip_eof(UNUSED void *pack, fs_file_t *file) {
    zip_file_t *zipfile = (zip_file_t *)file->handle;
    return zipfile->uncomp_pos >= zipfile->entry->uncomp_size;
}

fs_packtype_t fs_packtype_zip = {
    "zip",
    pack_zip_mount,
    pack_zip_unmount,
    fs_dirtree_walk,
    pack_zip_is_file,
    pack_zip_is_dir,
    pack_zip_open,
    pack_zip_read,
    pack_zip_seek,
    pack_zip_tell,
    pack_zip_size,
    pack_zip_eof,
    pack_zip_close,
};
