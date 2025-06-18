#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>

#define MAX_URLS 10
#define MAX_REFERERS 10

typedef struct {
    char* key;
    long value;
} HashEntry;

typedef struct {
    HashEntry** url_map;
    int url_map_size;
    int url_map_capacity;

    HashEntry** ref_map;
    int ref_map_size;
    int ref_map_capacity;

    long total_bytes;
} ThreadStats;

HashEntry** add_entry(HashEntry** map, int* size, int* capacity, const char* key, long delta) {
    for (int i = 0; i < *size; ++i) {
        if (strcmp(map[i]->key, key) == 0) {
            map[i]->value += delta;
            return map;
        }
    }

    if (*size >= *capacity) {
        *capacity = (*capacity == 0) ? 16 : *capacity * 2;
        map = realloc(map, (*capacity) * sizeof(HashEntry*));
    }

    HashEntry* entry = malloc(sizeof(HashEntry));
    entry->key = strdup(key);
    entry->value = delta;
    map[(*size)++] = entry;

    return map;
}

void add_url(ThreadStats* stats, const char* key, long delta) {
    stats->url_map = add_entry(stats->url_map, &stats->url_map_size, &stats->url_map_capacity, key, delta);
}

void add_ref(ThreadStats* stats, const char* key, long delta) {
    stats->ref_map = add_entry(stats->ref_map, &stats->ref_map_size, &stats->ref_map_capacity, key, delta);
}

int compare_by_value(const void* a, const void* b) {
    return (*(HashEntry**)b)->value - (*(HashEntry**)a)->value;
}

void select_top_n(HashEntry** map, int size, int n) {
    qsort(map, size, sizeof(HashEntry*), compare_by_value);
    for (int i = 0; i < n && i < size; ++i) {
        printf("%s : %ld\n", map[i]->key, map[i]->value);
    }
}

void parse_log_line(const char* line, ThreadStats* stats) {
    const char* ptr = strchr(line, '"');
    if (!ptr) return;
    ptr++; // skip "

    if (strncmp(ptr, "GET ", 4) != 0) return;
    ptr += 4;
    const char* url_end = strchr(ptr, ' ');
    if (!url_end) return;

    size_t url_len = url_end - ptr;
    char url[url_len + 1];
    strncpy(url, ptr, url_len);
    url[url_len] = '\0';

    if (url_len > 1 && url[url_len - 1] == '/') {
        url[url_len - 1] = '\0';
    }

    const char* size_ptr = strchr(url_end, '"');
    if (!size_ptr) return;
    size_ptr = strchr(size_ptr + 1, ' ');
    if (!size_ptr) return;

    int status = 0;
    long size = 0;
    if (sscanf(size_ptr, " %d %ld", &status, &size) != 2) return;

    stats->total_bytes += size;
    add_url(stats, url, size);

    const char* ref_start = strchr(size_ptr, '"');
    if (!ref_start) return;
    ref_start++;

    const char* ref_end = strchr(ref_start, '"');
    if (!ref_end || ref_end == ref_start) return;

    size_t ref_len = ref_end - ref_start;
    if (ref_len > 0) {
        char ref[ref_len + 1];
        strncpy(ref, ref_start, ref_len);
        ref[ref_len] = '\0';
        add_ref(stats, ref, 1);
    }
}

void process_file(const char* filename, ThreadStats* stats) {
    FILE* f = fopen(filename, "r");
    if (!f) return;

    char buffer[8192];
    while (fgets(buffer, sizeof(buffer), f)) {
        parse_log_line(buffer, stats);
    }

    fclose(f);
}

typedef struct {
    char** filenames;
    ThreadStats* stats;
} ThreadArg;

void* thread_function(void* arg) {
    ThreadArg* targ = (ThreadArg*)arg;
    for (int i = 0; targ->filenames[i]; ++i) {
        process_file(targ->filenames[i], targ->stats);
        free(targ->filenames[i]);
    }
    free(targ->filenames);
    return targ->stats;
}

char*** split_files(DIR* dir, const char* path, int nthreads) {
    char*** buckets = calloc(nthreads, sizeof(char**));
    int* counts = calloc(nthreads, sizeof(int));
    struct dirent* entry;
    int index = 0;

    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG) {
            int tid = index++ % nthreads;
            int count = counts[tid];

            char* full_path = malloc(strlen(path) + strlen(entry->d_name) + 2);
            sprintf(full_path, "%s/%s", path, entry->d_name);

            buckets[tid] = realloc(buckets[tid], (count + 2) * sizeof(char*));
            buckets[tid][count] = full_path;
            buckets[tid][count + 1] = NULL;
            counts[tid]++;
        }
    }
    free(counts);
    return buckets;
}

void merge_maps(HashEntry*** dest_map, int* dest_size, int* dest_cap, HashEntry** src_map, int src_size) {
    for (int i = 0; i < src_size; ++i) {
        *dest_map = add_entry(*dest_map, dest_size, dest_cap, src_map[i]->key, src_map[i]->value);
    }
}

void free_map(HashEntry** map, int size) {
    for (int i = 0; i < size; ++i) {
        free(map[i]->key);
        free(map[i]);
    }
    free(map);
}

void print_usage() {
    printf("Usage: log_parser <log_dir> <num_threads>\n");
}

int main(int argc, char** argv) {
    if (argc != 3) {
        print_usage();
        return 1;
    }

    int nthreads = atoi(argv[2]);
    if (nthreads <= 0) {
        fprintf(stderr, "Invalid thread count.\n");
        return 1;
    }

    DIR* dir = opendir(argv[1]);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    char*** files = split_files(dir, argv[1], nthreads);
    closedir(dir);

    pthread_t threads[nthreads];
    ThreadStats* results[nthreads];

    for (int i = 0; i < nthreads; ++i) {
        if (!files[i]) {
            nthreads = i;
            break;
        }
        ThreadArg* targ = malloc(sizeof(ThreadArg));
        targ->filenames = files[i];
        targ->stats = calloc(1, sizeof(ThreadStats));
        pthread_create(&threads[i], NULL, thread_function, targ);
    }

    ThreadStats final_stats = {0};

    for (int i = 0; i < nthreads; ++i) {
        void* res;
        pthread_join(threads[i], &res);
        ThreadStats* ts = (ThreadStats*)res;

        final_stats.total_bytes += ts->total_bytes;
        merge_maps(&final_stats.url_map, &final_stats.url_map_size, &final_stats.url_map_capacity,
                   ts->url_map, ts->url_map_size);
        merge_maps(&final_stats.ref_map, &final_stats.ref_map_size, &final_stats.ref_map_capacity,
                   ts->ref_map, ts->ref_map_size);

        free_map(ts->url_map, ts->url_map_size);
        free_map(ts->ref_map, ts->ref_map_size);
        free(ts);
    }

    printf("\nTotal transferred bytes: %ld\n", final_stats.total_bytes);
    printf("\nTop heavy URLs:\n");
    select_top_n(final_stats.url_map, final_stats.url_map_size, MAX_URLS);
    printf("\nMost frequent Referers:\n");
    select_top_n(final_stats.ref_map, final_stats.ref_map_size, MAX_REFERERS);

    free_map(final_stats.url_map, final_stats.url_map_size);
    free_map(final_stats.ref_map, final_stats.ref_map_size);

    return 0;
}
