#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>

typedef struct HashBucket
{
    char *key;
    void *value;

    struct HashBucket *next;
} HashBucket;

typedef struct HashMap {
    size_t size;
    size_t count;
    size_t elsize;  /* element size */

    HashBucket **buckets;  
} HashMap;

HashMap *HashMapCreate(size_t elsize);
void HashMapDestroy(HashMap *hashmap);
void HashMapSet(HashMap *hashmap, const char *key, const void *value);
void *HashMapGet(HashMap *hashmap, const char *key);
void HashMapClear(HashMap *hashmap, const char *key);

#endif