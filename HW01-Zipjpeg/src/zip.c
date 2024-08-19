#include "zip.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "detail.h"

#define READ16(p) ((p) += 2, read16le((p) - 2))
#define READ32(p) ((p) += 4, read32le((p) - 4))

/* Size of the End of Central Directory Record, not including comment. */
#define EOCDR_BASE_SZ 22
#define EOCDR_SIGNATURE 0x06054b50  /* "PK\5\6" little-endian. */

#define CFH_BASE_SZ 46
#define CFH_SIGNATURE 0x02014b50 /* "PK\1\2" little-endian. */

#define LFH_BASE_SZ 30
#define LFH_SIGNATURE 0x04034b50 /* "PK\3\4" little-endian. */

#define EXT_ATTR_DIR (1U << 4)
#define EXT_ATTR_ARC (1U << 5)

size_t find_eocdr(struct eocdr *r, const uint8_t *src, size_t src_len)
{
    size_t comment_len;
    const uint8_t *p;
    uint32_t signature;

    for (comment_len = 0; comment_len <= UINT16_MAX; comment_len++) {
        if (src_len < EOCDR_BASE_SZ + comment_len) {
            break;
        }
        size_t offset = src_len - EOCDR_BASE_SZ - comment_len;
        p = &src[offset];
        signature = READ32(p);

        if (signature == EOCDR_SIGNATURE) {
            r->disk_nbr = READ16(p);
            r->cd_start_disk = READ16(p);
            r->disk_cd_entries = READ16(p);
            r->cd_entries = READ16(p);
            r->cd_size = READ32(p);
            r->cd_offset = READ32(p);
            r->comment_len = READ16(p);
            r->comment = p;
            assert(p == &src[src_len - comment_len] &&
                   "All fields read.");

            if (r->comment_len == comment_len) {
                    return offset;
            }
        }
    }

    return 0;
}


_Bool read_cfh(struct cfh *cfh, const uint8_t *src, size_t src_len, size_t offset)
{
    const uint8_t *p;
    uint32_t signature;

    if (offset > src_len || src_len - offset < CFH_BASE_SZ) {
        return 0;
    }

    p = &src[offset];
    signature = READ32(p);
    if (signature != CFH_SIGNATURE) {
        return 0;
    }

    cfh->made_by_ver = READ16(p);
    cfh->extract_ver = READ16(p);
    cfh->gp_flag = READ16(p);
    cfh->method = READ16(p);
    cfh->mod_time = READ16(p);
    cfh->mod_date = READ16(p);
    cfh->crc32 = READ32(p);
    cfh->comp_size = READ32(p);
    cfh->uncomp_size = READ32(p);
    cfh->name_len = READ16(p);
    cfh->extra_len = READ16(p);
    cfh->comment_len = READ16(p);
    cfh->disk_nbr_start = READ16(p);
    cfh->int_attrs = READ16(p);
    cfh->ext_attrs = READ32(p);
    cfh->lfh_offset = READ32(p);
    cfh->name = p;
    cfh->extra = cfh->name + cfh->name_len;
    cfh->comment = cfh->extra + cfh->extra_len;
    assert(p == &src[offset + CFH_BASE_SZ] && "All fields read.");

    if ((src_len - offset - CFH_BASE_SZ) < (size_t)(cfh->name_len + cfh->extra_len + cfh->comment_len)) {
        return 0;
    }

    return 1;
}

_Bool read_lfh(struct lfh *lfh, const uint8_t *src, size_t src_len, size_t offset)
{
    const uint8_t *p;
    uint32_t signature;

    if (offset > src_len || src_len - offset < LFH_BASE_SZ) {
        return 0;
    }

    p = &src[offset];
    signature = READ32(p);
    if (signature != LFH_SIGNATURE) {
        return 0;
    }

    lfh->extract_ver = READ16(p);
    lfh->gp_flag = READ16(p);
    lfh->method = READ16(p);
    lfh->mod_time = READ16(p);
    lfh->mod_date = READ16(p);
    lfh->crc32 = READ32(p);
    lfh->comp_size = READ32(p);
    lfh->uncomp_size = READ32(p);
    lfh->name_len = READ16(p);
    lfh->extra_len = READ16(p);
    lfh->name = p;
    lfh->extra = lfh->name + lfh->name_len;
    assert(p == &src[offset + LFH_BASE_SZ] && "All fields read.");

    if (src_len - offset - LFH_BASE_SZ < lfh->name_len + lfh->extra_len) {
        return 0;
    }

    return 1;
}

/* Initialize zip based on the source data. Returns true on success, or false
   if the data could not be parsed as a valid Zip file. */
_Bool zip_read(zip_t *zip, const uint8_t *src, size_t src_len)
{
    struct eocdr eocdr;
    struct cfh cfh;
    struct lfh lfh;
    size_t i, offset;
    const uint8_t *comp_data;

    size_t eocdr_offset = find_eocdr(&eocdr, src, src_len);

    if (!eocdr_offset) {
        return 0;
    }

    zip->src_len = eocdr.cd_offset + eocdr.cd_size + src_len - eocdr_offset;
    zip->src = &src[src_len - zip->src_len];

    //size_t offset_diff = eocdr_offset - eocdr.cd_size - eocdr.cd_offset;

    if (eocdr.disk_nbr != 0 || eocdr.cd_start_disk != 0 ||
        eocdr.disk_cd_entries != eocdr.cd_entries) {
        return 0; /* Cannot handle multi-volume archives. */
    }

    zip->num_members = eocdr.cd_entries;
    zip->comment = eocdr.comment;
    zip->comment_len = eocdr.comment_len;

    offset = eocdr.cd_offset;
    zip->members_begin = offset;

    /* Read the member info and do a few checks. */
    for (i = 0; i < eocdr.cd_entries; i++) {
        if (!read_cfh(&cfh, zip->src, zip->src_len, offset)) {
            return 0;
        }

        if (cfh.gp_flag & 1) {
            return 0; /* The member is encrypted. */
        }
        if (cfh.method != ZIP_STORED && cfh.method != ZIP_DEFLATED) {
            return 0; /* Unsupported compression method. */
        }
        if (cfh.method == ZIP_STORED &&
            cfh.uncomp_size != cfh.comp_size) {
            return 0;
        }
        if (cfh.disk_nbr_start != 0) {
            return 0; /* Cannot handle multi-volume archives. */
        }
        if (memchr(cfh.name, '\0', cfh.name_len) != NULL) {
            return 0; /* Bad filename. */
        }

        if (!read_lfh(&lfh, zip->src, zip->src_len, cfh.lfh_offset)) {
            return 0;
        }

        comp_data = lfh.extra + lfh.extra_len;
        if (cfh.comp_size > zip->src_len - (size_t)(comp_data - zip->src)) {
            return 0; /* Member data does not fit in src. */
        }

        offset += CFH_BASE_SZ + cfh.name_len + cfh.extra_len +
                  cfh.comment_len;
    }

    zip->members_end = offset;

    return 1;
}

/* Get the Zip archive member through iterator it. */
zipmemb_t zip_member(const zip_t *zip, zipiter_t it)
{
    struct cfh cfh;
    struct lfh lfh;
    _Bool ok;
    zipmemb_t m;

    assert(it >= zip->members_begin && it < zip->members_end);

    ok = read_cfh(&cfh, zip->src, zip->src_len, it);
    assert(ok);

    ok = read_lfh(&lfh, zip->src, zip->src_len, cfh.lfh_offset);
    assert(ok);

    m.name = cfh.name;
    m.name_len = cfh.name_len;
    m.mtime = dos2ctime(cfh.mod_date, cfh.mod_time);
    m.comp_size = cfh.comp_size;
    m.comp_data = lfh.extra + lfh.extra_len;
    m.method = cfh.method;
    m.uncomp_size = cfh.uncomp_size;
    m.crc32 = cfh.crc32;
    m.comment = cfh.comment;
    m.comment_len = cfh.comment_len;
    m.is_dir = (cfh.ext_attrs & EXT_ATTR_DIR) != 0;

    m.next = it + CFH_BASE_SZ + cfh.name_len + cfh.extra_len + cfh.comment_len;

    assert(m.next <= zip->members_end);

    return m;
}

void list_zip(const char *filename)
{
    uint8_t *zip_data;
    size_t zip_sz;
    zip_t z;
    zipiter_t it;
    zipmemb_t m;

    printf("Listing ZIP archive: %s\n\n", filename);

    zip_data = read_file(filename, &zip_sz);

    if (!zip_read(&z, zip_data, zip_sz)) {
        printf("Failed to parse ZIP file!\n");
        exit(1);
    }

    if (z.comment_len != 0) {
        printf("%.*s\n\n", (int)z.comment_len, z.comment);
    }

    for (it = z.members_begin; it != z.members_end; it = m.next) {
        m = zip_member(&z, it);
        printf("%.*s\n", (int)m.name_len, m.name);
    }

    printf("\n");

    free(zip_data);
}
