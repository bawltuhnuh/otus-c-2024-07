#include <stdlib.h>
#include <stdio.h>

#include "detail.h"

#define PERROR_IF(cond, msg) if (cond) { perror(msg); exit(1); }

void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    PERROR_IF(ptr == NULL, "malloc");
    return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
    ptr = realloc(ptr, size);
    PERROR_IF(ptr == NULL, "realloc");
    return ptr;
}

uint8_t *read_file(const char *filename, size_t *file_sz)
{
    FILE *f;
    uint8_t *buf;
    size_t buf_cap;

    f = fopen(filename, "rb");
    PERROR_IF(f == NULL, "fopen");

    buf_cap = 4096;
    buf = xmalloc(buf_cap);

    *file_sz = 0;
    while (feof(f) == 0) {
            if (buf_cap - *file_sz == 0) {
                    buf_cap *= 2;
                    buf = xrealloc(buf, buf_cap);
            }

            *file_sz += fread(&buf[*file_sz], 1, buf_cap - *file_sz, f);
            PERROR_IF(ferror(f), "fread");
    }

    PERROR_IF(fclose(f) != 0, "fclose");
    return buf;
}

time_t dos2ctime(uint16_t dos_date, uint16_t dos_time)
{
        struct tm tm = {0};

        tm.tm_sec = (dos_time & 0x1f) * 2;  /* Bits 0--4:  Secs divided by 2. */
        tm.tm_min = (dos_time >> 5) & 0x3f; /* Bits 5--10: Minute. */
        tm.tm_hour = (dos_time >> 11);      /* Bits 11-15: Hour (0--23). */

        tm.tm_mday = (dos_date & 0x1f);          /* Bits 0--4: Day (1--31). */
        tm.tm_mon = ((dos_date >> 5) & 0xf) - 1; /* Bits 5--8: Month (1--12). */
        tm.tm_year = (dos_date >> 9) + 80;       /* Bits 9--15: Year-1980. */

        tm.tm_isdst = -1;

        return mktime(&tm);
}

uint32_t read32le(const uint8_t *p)
{
    return ((uint32_t)p[0] << 0)  |
           ((uint32_t)p[1] << 8)  |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

uint16_t read16le(const uint8_t *p)
{
    return (uint16_t)(
            ((uint16_t)p[0] << 0) |
            ((uint16_t)p[1] << 8));
}
