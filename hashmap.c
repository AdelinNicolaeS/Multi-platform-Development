#include "hashmap.h"
#define INITIAL_SIZE_MAP 1000

// from stack overflow
unsigned long hash(char *str)
{
	unsigned long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

hashmap *init_hashmap(void)
{
	hashmap *map = calloc(1, sizeof(hashmap));

	if (map == NULL)
		exit(12);
	map->maximum_capacity = INITIAL_SIZE_MAP;
	map->current_capacity = 0;
	map->elements = calloc(map->maximum_capacity, sizeof(pair *));
	if (map->elements == NULL)
		exit(12);
	return map;
}

void resize_hashmap(hashmap *map)
{
	map->maximum_capacity *= 4;
	map->elements = realloc(map->elements, map->maximum_capacity);
	if (map->elements == NULL)
		exit(12);
}

void add_pair(hashmap *map, char *key, char *value)
{
	int index = hash(key) % (map->maximum_capacity);
	int i;

	if (map->elements[index] == NULL) {
		/* no collision, put the element */
		map->elements[index] = malloc(sizeof(pair));
		if (map->elements[index] == NULL)
			exit(12);
		map->elements[index]->key =
		    calloc(strlen(key) + 1, sizeof(char));
		if (map->elements[index]->key == NULL)
			exit(12);
		map->elements[index]->value =
		    calloc(strlen(value) + 1, sizeof(char));
		if (map->elements[index]->value == NULL)
			exit(12);
		strcpy(map->elements[index]->key, key);
		strcpy(map->elements[index]->value, value);
		map->current_capacity++;
	} else if (strcmp(map->elements[index]->key, key) == 0) {
		map->elements[index]->value =
		    realloc(map->elements[index]->value,
			    (strlen(value) + 1) * sizeof(char));
		if (map->elements[index]->value == NULL)
			exit(12);
		strcpy(map->elements[index]->value, value);
	} else {
		/* different key, collision */
		/* put element in the first empty space */
		int found_empty_space = 0;

		for (i = 1; i < map->maximum_capacity; i++) {
			int current_index =
			    (index + i) % (map->maximum_capacity);

			if (map->elements[current_index] == NULL) {
				/* find empty space */
				map->elements[current_index] =
				    malloc(sizeof(pair));
				if (map->elements[current_index] == NULL)
					exit(12);
				map->elements[current_index]->key =
				    calloc(sizeof(key) + 1, sizeof(char));
				if (map->elements[current_index]->key == NULL)
					exit(12);
				map->elements[current_index]->value =
				    calloc(sizeof(value) + 1, sizeof(char));
				if (map->elements[current_index]->value ==
				    NULL)
					exit(12);
				strcpy(map->elements[current_index]->key, key);
				strcpy(map->elements[current_index]->value,
				       value);
				found_empty_space = 1;
				map->current_capacity++;
				break;
			}
		}
		if (found_empty_space == 0) {
			resize_hashmap(map);
			add_pair(map, key, value);
		}
	}
}

char *get_value(hashmap *map, char *key)
{
	int index = hash(key) % map->maximum_capacity;
	int current_index, i;

	for (i = 0; i < map->maximum_capacity; i++) {
		current_index = (index + i) % map->maximum_capacity;

		if (map->elements[current_index] != NULL &&
		    map->elements[current_index]->key != NULL &&
		    strcmp(map->elements[current_index]->key, key) == 0) {
			return map->elements[current_index]->value;
		}
	}
	return NULL;
}

void delete_key(hashmap *map, char *key)
{
	int index = hash(key) % map->maximum_capacity;
	int current_index, i;

	for (i = 0; i < map->maximum_capacity; i++) {
		current_index = (index + i) % map->maximum_capacity;

		if (map->elements[current_index] != NULL &&
		    map->elements[current_index]->key != NULL &&
		    strcmp(map->elements[current_index]->key, key) == 0) {
			map->current_capacity--;
			free(map->elements[current_index]->key);
			free(map->elements[current_index]->value);
			free(map->elements[current_index]);
			map->elements[current_index] = NULL;
			break;
		}
	}
}

void print_map(hashmap *map)
{
	int i;

	printf("Capacity hashmap: %d\n", map->current_capacity);
	for (i = 0; i < map->maximum_capacity; i++) {
		if (map->elements[i] != NULL) {
			printf("(key, value) = (%s, %s)\n",
			       map->elements[i]->key, map->elements[i]->value);
		}
	}
}

void delete_map(hashmap *map)
{
	int i;

	for (i = 0; i < map->maximum_capacity; i++) {
		if (map->elements[i] != NULL) {
			if (map->elements[i]->key != NULL)
				free(map->elements[i]->key);
			if (map->elements[i]->value != NULL)
				free(map->elements[i]->value);
			free(map->elements[i]);
			map->elements[i] = NULL;
		}
	}
	free(map->elements);
	free(map);
}
