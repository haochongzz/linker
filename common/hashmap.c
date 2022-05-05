#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hashmap.h"

#define INIT_SIZE (1024)
#define GROWTH_FACTOR (2)
#define LOAD_FACTOR (1)

static unsigned long Hash(const char *key)
{
    unsigned long p = 31;
    unsigned long m = 1000000007;

    unsigned long k = p;
    unsigned long v = 0;
    for (int i = 0; i < strlen(key); ++ i)
    {
        v = (v + ((unsigned long)key[i] * k) % m) % m;
        k = (k * p) % m;
    }

    return v;
}

HashMap *HashMapCreate(size_t elsize)
{
    /* allocate memory for hashmap */
    HashMap *hashmap = malloc(sizeof(HashMap));

    /* init hashmap */
    hashmap->size = INIT_SIZE;
    hashmap->count = 0;
    hashmap->elsize = elsize;
    hashmap->buckets = malloc(sizeof(HashBucket *) * INIT_SIZE);

    /* init buckets */
    for (int i = 0; i < INIT_SIZE; i ++)
    {
        hashmap->buckets[i] = NULL;
    }

    return hashmap;
}

void HashMapDestroy(HashMap *hashmap)
{
    size_t count = hashmap->count;

    for (size_t i = 0; i < hashmap->size; i ++)
    {
        HashBucket *b = hashmap->buckets[i];
        HashBucket *next;
        for (; b != NULL; b = next)
        {
            next = b->next;
            count --;

            free(b->key);
            free(b->value);
            free(b);
        }

        /* break the loop if hashmap is empty */
        if (!count) break;
    }

    free(hashmap->buckets);
    free(hashmap);
}

HashBucket *HashBucketNew(const char *key, const void *value, size_t elsize)
{
    HashBucket *bucket = malloc(sizeof(HashBucket));
    bucket->key = malloc(strlen(key) + 1);
    bucket->value = malloc(elsize);
    bucket->next = NULL;

    memcpy(bucket->key, key, strlen(key) + 1);
    memcpy(bucket->value, value, elsize);

    return bucket;
}

void HashMapResize(HashMap *hashmap)
{
    HashMap *temp = malloc(sizeof(HashMap));
    temp->size = hashmap->size * 2;
    size_t mask = temp->size - 1;

    temp->buckets = malloc(sizeof(HashBucket *) * temp->size);
    for (size_t i = 0; i < temp->size; i ++)
    {
        temp->buckets[i] = NULL;
    }

    for (size_t i = 0; i < hashmap->size; i ++)
    {
        HashBucket *b = hashmap->buckets[i];
        HashBucket *next;
        for (; b != NULL; b = next)
        {
            size_t index = Hash(b->key) & mask;
            HashBucket **pptr = &temp->buckets[index];
            while (*pptr != NULL)
            {
                pptr = &(*pptr)->next;
            }
            *pptr = b;
            next = b->next;
            b->next = NULL;
        }
    }

    free(hashmap->buckets);
    hashmap->buckets = temp->buckets;
    hashmap->size = temp->size;
    free(temp);
}

void HashMapSet(HashMap *hashmap, const char *key, const void *value)
{
    size_t mask = hashmap->size - 1;
    size_t index = Hash(key) & mask;

    HashBucket **b = &hashmap->buckets[index];
    while (*b != NULL)
    {
        HashBucket *bucket = *b;
        if (strcmp(key, bucket->key) == 0)
        {
            /* match found and replace the value */
            free(bucket->value);
            bucket->value = malloc(hashmap->elsize);
            memcpy(bucket->value, value, hashmap->elsize);
            return;
        }
        b = &bucket->next;
    }
    /* the end of linked list reached without a match, add new bucket */
    *b = HashBucketNew(key, value, hashmap->elsize);

    hashmap->count ++;

    /* resize hashmap if count reach the load threshold */
    if (hashmap->count >= (hashmap->size * LOAD_FACTOR))
    {
        HashMapResize(hashmap);
    }
}

void *HashMapGet(HashMap *hashmap, const char *key)
{
    size_t mask = hashmap->size - 1;
    size_t index = Hash(key) & mask;

    HashBucket *bucket = hashmap->buckets[index];
    while (bucket != NULL)
    {
        if (strcmp(key, bucket->key) == 0)
        {
            return bucket->value;
        }
        bucket = bucket->next;
    }

    return NULL;
 }

 void HashMapClear(HashMap *hashmap, const char *key)
 {
    size_t mask = hashmap->size - 1;
    size_t index = Hash(key) & mask;

    HashBucket **b = &hashmap->buckets[index];
    while (b != NULL)
    {
        HashBucket *bucket = *b;
        if (strcmp(key, bucket->key) == 0)
        {
            free(bucket->key);
            free(bucket->value);
            (*b) = bucket->next;
            free(bucket);
            hashmap->count --;
            return;
        }
        b = &(*b)->next;
    }
 }