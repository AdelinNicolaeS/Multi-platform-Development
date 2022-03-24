#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct pair {
	char *key;
	char *value;
} pair;

typedef struct hashmap {
	int current_capacity;
	int maximum_capacity;

	pair **elements;

} hashmap;

unsigned long hash(char *str);
hashmap *init_hashmap(void);
void resize_hashmap(hashmap *map);
void add_pair(hashmap *map, char *key, char *value);
void print_map(hashmap *map);
char *get_value(hashmap *map, char *key);
void delete_key(hashmap *map, char *key);
void delete_map(hashmap *map);
