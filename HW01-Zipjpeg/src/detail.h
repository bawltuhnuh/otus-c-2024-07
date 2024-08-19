#ifndef DETAIL_H
#define DETAIL_H

#include <stddef.h>
#include <inttypes.h>
#include <time.h>

void *xmalloc(size_t size);

void *xrealloc(void *ptr, size_t size);

uint8_t *read_file(const char *filename, size_t *file_sz);

time_t dos2ctime(uint16_t dos_date, uint16_t dos_time);

uint32_t read32le(const uint8_t *p);

uint16_t read16le(const uint8_t *p);

#endif // DETAIL_H
