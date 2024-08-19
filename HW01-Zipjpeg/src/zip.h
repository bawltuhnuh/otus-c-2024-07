#ifndef ZIP_H
#define ZIP_H

#include <inttypes.h>
#include <stddef.h>
#include <time.h>

typedef size_t zipiter_t; /* Zip archive member iterator. */

typedef struct zip_t zip_t;

typedef enum { ZIP_STORED = 0, ZIP_DEFLATED = 8 } method_t;

typedef struct zipmemb_t zipmemb_t;

struct eocdr {
        uint16_t disk_nbr;        /* Number of this disk. */
        uint16_t cd_start_disk;   /* Nbr. of disk with start of the CD. */
        uint16_t disk_cd_entries; /* Nbr. of CD entries on this disk. */
        uint16_t cd_entries;      /* Nbr. of Central Directory entries. */
        uint32_t cd_size;         /* Central Directory size in bytes. */
        uint32_t cd_offset;       /* Central Directory file offset. */
        uint16_t comment_len;     /* Archive comment length. */
        const uint8_t *comment;   /* Archive comment. */
};

struct cfh {
        uint16_t made_by_ver;    /* Version made by. */
        uint16_t extract_ver;    /* Version needed to extract. */
        uint16_t gp_flag;        /* General purpose bit flag. */
        uint16_t method;         /* Compression method. */
        uint16_t mod_time;       /* Modification time. */
        uint16_t mod_date;       /* Modification date. */
        uint32_t crc32;          /* CRC-32 checksum. */
        uint32_t comp_size;      /* Compressed size. */
        uint32_t uncomp_size;    /* Uncompressed size. */
        uint16_t name_len;       /* Filename length. */
        uint16_t extra_len;      /* Extra data length. */
        uint16_t comment_len;    /* Comment length. */
        uint16_t disk_nbr_start; /* Disk nbr. where file begins. */
        uint16_t int_attrs;      /* Internal file attributes. */
        uint32_t ext_attrs;      /* External file attributes. */
        uint32_t lfh_offset;     /* Local File Header offset. */
        const uint8_t *name;     /* Filename. */
        const uint8_t *extra;    /* Extra data. */
        const uint8_t *comment;  /* File comment. */
};

struct lfh {
        uint16_t extract_ver;
        uint16_t gp_flag;
        uint16_t method;
        uint16_t mod_time;
        uint16_t mod_date;
        uint32_t crc32;
        uint32_t comp_size;
        uint32_t uncomp_size;
        uint16_t name_len;
        uint16_t extra_len;
        const uint8_t *name;
        const uint8_t *extra;
};

struct zip_t {
        uint16_t num_members;    /* Number of members. */
        const uint8_t *comment;  /* Zip file comment (not terminated). */
        uint16_t comment_len;    /* Zip file comment length. */
        zipiter_t members_begin; /* Iterator to the first member. */
        zipiter_t members_end;   /* Iterator to the end of members. */

        const uint8_t *src;
        size_t src_len;
};


struct zipmemb_t {
        const uint8_t *name;      /* Member name (not null terminated). */
        uint16_t name_len;        /* Member name length. */
        time_t mtime;             /* Modification time. */
        uint32_t comp_size;       /* Compressed size. */
        const uint8_t *comp_data; /* Compressed data. */
        method_t method;          /* Compression method. */
        uint32_t uncomp_size;     /* Uncompressed size. */
        uint32_t crc32;           /* CRC-32 checksum. */
        const uint8_t *comment;   /* Comment (not null terminated). */
        uint16_t comment_len;     /* Comment length. */
        _Bool is_dir;              /* Whether this is a directory. */
        zipiter_t next;           /* Iterator to the next member. */
};

size_t find_eocdr(struct eocdr *r, const uint8_t *src, size_t src_len);

_Bool read_cfh(struct cfh *cfh, const uint8_t *src, size_t src_len, size_t offset);

_Bool read_lfh(struct lfh *lfh, const uint8_t *src, size_t src_len, size_t offset);

zipmemb_t zip_member(const zip_t *zip, zipiter_t it);

void list_zip(const char *filename);

#endif // ZIP_H
