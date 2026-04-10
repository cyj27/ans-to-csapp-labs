#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
typedef struct {
    int valid;
    int tag;
    int lru_stamp;
} cache_line;
typedef struct {
    cache_line *lines;
} cache_set;
typedef struct {
    cache_set *sets;
} Cache;
int s, E, b = 0;
char *trace_file = NULL;
int verbose = 0;

int hits = 0;
int misses = 0;
int evictions = 0;
unsigned long long global_time = 0;
Cache cache;

void cache_access(int set_index, int tag);

int main(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "s:E:b:t:hv")) != -1) {
        switch (opt) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
                printf("Usage: %s [-v] -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
                exit(0);
            default:
                printf("Invalid argument.\n");
                exit(1);
        }
    }
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("Missing required arguments.\n");
        exit(1);
    }
    cache.sets = (cache_set *)malloc(sizeof(cache_set) * (1 << s));
    for (int i = 0; i < (1 << s); i++) {
        cache.sets[i].lines = (cache_line *)malloc(sizeof(cache_line) * E);
        for (int j = 0; j < E; j++) {
            cache.sets[i].lines[j].valid = 0;
            cache.sets[i].lines[j].tag = 0;
            cache.sets[i].lines[j].lru_stamp = 0;
        }
    }
    FILE *file = fopen(trace_file, "r");
    if (file == NULL) {
        printf("Error opening trace file.");
        exit(1);
    }
    char operation;
    unsigned long long add;
    int size;
    while (fscanf(file, " %c %llx,%d", &operation, &add, &size) == 3) {
        if (operation == 'I') {
            continue;
        }
        global_time++;
        int set_index = (add >> b) & ((1 << s) - 1);
        int tag = add >> (b + s);
        cache_access(set_index, tag);
        if (operation == 'M') hits++;
    }
    fclose(file);
    for (int i = 0; i < (1 << s); i++) {
        free(cache.sets[i].lines);
    }
    free(cache.sets);
    printSummary(hits, misses, evictions);
    return 0;
}
void cache_access(int set_index, int tag){
    cache_set target_set = cache.sets[set_index];
    int hit_flag = 0;
    int empty_line_index = -1;
    int evict_line_index = 0;
    unsigned long long min_stamp = target_set.lines[0].lru_stamp;
    for (int i = 0; i < E; i++) {
        if (target_set.lines[i].valid == 1 && target_set.lines[i].tag == tag) {
            hits++;
            hit_flag = 1;
            target_set.lines[i].lru_stamp = global_time;
            break;
        }
        if (target_set.lines[i].valid == 0 && empty_line_index == -1) {
            empty_line_index = i;
        }
        if (target_set.lines[i].valid == 1 && target_set.lines[i].lru_stamp < min_stamp) {
            min_stamp = target_set.lines[i].lru_stamp;
            evict_line_index = i;
        }
    }
    if (hit_flag == 0) {
        misses++;
        if (empty_line_index != -1) {
            target_set.lines[empty_line_index].valid = 1;
            target_set.lines[empty_line_index].tag = tag;
            target_set.lines[empty_line_index].lru_stamp = global_time;
        } else {
            evictions++;
            target_set.lines[evict_line_index].tag = tag;
            target_set.lines[evict_line_index].lru_stamp = global_time;
        }
    }
}
