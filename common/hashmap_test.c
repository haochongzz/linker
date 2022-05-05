#include <stdio.h>
#include <stdlib.h>

#include "hashmap.h"

typedef struct MyStruct
{
    int value;
} MyStruct;

int main(int argc, char **argv)
{
    HashMap *map = HashMapCreate(sizeof(MyStruct));

    MyStruct a, b, c;
    a.value = 1;
    b.value = 2;
    c.value = 3;

    HashMapSet(map, "a", &a);
    HashMapSet(map, "b", &b);
    HashMapSet(map, "c", &c);

    MyStruct *v = HashMapGet(map, "a");
    if (v == NULL)
    {
        printf("cann't find key %s\n", "a");
    }
    printf("key %s, get value: %d\n", "a", v->value);
    v = HashMapGet(map, "b");
    if (v == NULL)
    {
        printf("cann't find key %s\n", "b");
    }
    printf("key %s, get value: %d\n", "b", v->value);
    v = HashMapGet(map, "c");
    if (v == NULL)
    {
        printf("cann't find key %s\n", "c");
    }
    printf("key %s, get value: %d\n", "c", v->value);

    HashMapClear(map, "a");
    v = HashMapGet(map, "a");
    if (v == NULL)
    {
        printf("cann't find key %s\n", "a");
    }

    HashMapDestroy(map);

    return EXIT_SUCCESS;
}