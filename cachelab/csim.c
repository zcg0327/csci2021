/*
 * csim.c - A cache simulator that can replay traces from Valgrind
 *     and output statistics such as number of hits, misses, and
 *     evictions.  The replacement policy is MRU.
 *
 * modified by Brady Olson
 * December 10, 2016
 *
 * The function printSummary() is given to print output.
 * Please use this function to print the number of hits, misses and evictions.
 * This is crucial for the driver to evaluate your work. 
 */
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"

//#define DEBUG_ON 
#define ADDRESS_LENGTH 64

/* Type: Memory address */
typedef unsigned long long int mem_addr_t;

/* Type: Cache line
   MRU is a counter used to implement MRU replacement policy  */
typedef struct cache_line {
    char valid;
    mem_addr_t tag;
    unsigned long long int mru;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

/* Globals set by command line args */
int verbosity = 0; /* print trace if set */
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int E = 0; /* associativity */
char* trace_file = NULL;

/* Derived from command line args */
int S; /* number of sets */
int B; /* block size (bytes) */

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int mru_counter = 1;

/* The cache we are simulating */
cache_t cache;  
mem_addr_t set_index_mask;

/* 
 * initCache - Allocate memory, write 0's for valid and tag and MRU
 * also computes the set_index_mask
 */
void initCache() {
    set_index_mask = S - 1;

    /* allocate space for cache */
    cache = malloc(S * sizeof(cache_t*));
    for (int currentSet = 0; currentSet < S; currentSet++) {
        /* allocate space for each cache line */
        cache[currentSet] = malloc(E * sizeof(cache_line_t));
        for (int currentLine = 0; currentLine < E; currentLine++) {
            /* initialize all valid bits, tags, and MRU count to 0 */
            cache[currentSet][currentLine].valid = 0;
            cache[currentSet][currentLine].tag = 0;
            cache[currentSet][currentLine].mru = 0;
        }   
    }   
}


/* 
 * freeCache - free allocated memory
 */
void freeCache()
{
    int i;
    for (i=0; i<S; i++){
        free(cache[i]);
    }
    free(cache);
}


/* 
 * accessData - Access data at memory address addr.
 *   If it is already in cache, increast hit_count
 *   If it is not in cache, bring it in cache, increase miss count.
 *   Also increase eviction_count if a line is evicted.
 */
void accessData(mem_addr_t addr) {
	/* initialize largest MRU value counter*/
    long long largestMRUval = 0;
    /* initialize largest MRU index */
    long largestMRUindex;
    /* get set index and bitwise-and with mask */
    long long currentSet = (addr >> b) & set_index_mask;
    /* get cache tag */
    long currentTag = (addr >> (s + b));

	for(int currentLine = 0; currentLine < E; currentLine++) {
		/* if current line matches with tag and is valid,
			update hitcounter & mru.
			then return since no miss calculatino required. */
		if (cache[currentSet][currentLine].tag == currentTag && 
			cache[currentSet][currentLine].valid == 1) {
			hit_count++;
			cache[currentSet][currentLine].mru = mru_counter++;
			return;
		}
	}
	/* postcondition of loop:
		function did not return, so must be cache miss. */
	miss_count++;

	for(int currentLine = 0; currentLine < E; currentLine++) {
		/* if curernt line is empty,
			load block and update valid & mru.
			then return since no eviction required. */
		if(cache[currentSet][currentLine].mru == 0) {
			cache[currentSet][currentLine].tag = currentTag;
			cache[currentSet][currentLine].valid = 1;
			cache[currentSet][currentLine].mru = mru_counter++;
			return;
		}
		/* if current line has highest MRUval thus far,
			update largest MRU value counter and index. */
		if(cache[currentSet][currentLine].mru > largestMRUval) {
			largestMRUval = cache[currentSet][currentLine].mru;
			largestMRUindex = currentLine;
		}
	}
	/* postcondition of loop:
		function did not return so largestMRUindex holds
		the index of line with highest MRU value. */
	eviction_count++;
	/* line eviction based on above loop result. */
	cache[currentSet][largestMRUindex].tag = currentTag;
	cache[currentSet][largestMRUindex].valid = 1;
	cache[currentSet][largestMRUindex].mru = mru_counter++;
}

/*
 * replayTrace - replays the given trace file against the cache 
 */
void replayTrace(char* trace_fn)
{
    char buf[1000];
    mem_addr_t addr=0;
    unsigned int len=0;
    FILE* trace_fp = fopen(trace_fn, "r");

    if(!trace_fp){
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while(fgets(buf, 1000, trace_fp) != NULL) {
        /* buf[Y] gives the Yth byte in the trace line */

        /* Read address and length from the trace using sscanf 
         */
        if (buf[1] == 'M' || buf[1] == 'L' || buf[1] == 'S') {
            sscanf(buf + 3, "%llx, %u", &addr, &len);
            accessData(addr);
        }
         /*    ACCESS THE CACHE, i.e. CALL accessData */
        if (buf[1] == 'M') {
            accessData(addr);
        }
    }

    fclose(trace_fp);
}

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

/*
 * main - Main routine 
 */
int main(int argc, char* argv[])
{
    char c;

    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch(c){
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
            verbosity = 1;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }   
    }   

    /* Make sure that all required command line args were specified */
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }   

    /* Compute S, E and B from command line args */
    S = pow(2,s);
    B = pow(2,b);


    /* Initialize cache */
    initCache();

#ifdef DEBUG_ON
    printf("DEBUG: S:%u E:%u B:%u trace:%s\n", S, E, B, trace_file);
    printf("DEBUG: set_index_mask: %llu\n", set_index_mask);
#endif
    
    /* Read the trace and access the cache */
    replayTrace(trace_file);

    /* Free allocated memory */
    freeCache();

    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}

