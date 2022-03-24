#include "hashmap.h"
#include <stdio.h>

#define LINE_LENGTH 256

int find_no_directories(char **argv, int argc)
{
	int i, count = 0;

	for (i = 0; i < argc; i++)
		if (strncmp(argv[i], "-I", 2) == 0)
			count++;

	return count;
}

int equal_position(char *element)
{
	int i, n = strlen(element);

	for (i = 0; i < n; i++) {
		if (element[i] == '=')
			return i;
	}
	return -1;
}

void map_first_define(hashmap *map, char *brute)
{
	int length = strlen(brute);
	int eq_position = equal_position(brute);
	char *my_value = NULL;

	if (eq_position == -1) {
		add_pair(map, brute, "");
	} else {
		char *my_key = calloc((eq_position + 1), sizeof(char));

		if (my_key == NULL)
			exit(12);
		my_value = calloc(length - eq_position - 1 + 1, sizeof(char));
		if (my_value == NULL)
			exit(12);
		strncpy(my_key, brute, eq_position);
		sprintf(my_value, "%s", brute + eq_position + 1);
		add_pair(map, my_key, my_value);
		free(my_key);
		free(my_value);
	}
}

void map_parameters(hashmap *map, int argc, char *argv[], char **input,
		    char **output, FILE **in, FILE **out,
		    char **folders)
{
	int i = 1;
	int ignore_next_argv = 0;
	int pos_folder = -1;

	for (; i < argc; ++i) {
		if (ignore_next_argv == 1) {
			ignore_next_argv = 0;
			continue;
		}
		/* check each type of parameters (aka -{D, I, o}) */
		if (strncmp(argv[i], "-D", 2) == 0) {
			if (strlen(argv[i]) == 2) {
				/* mapping the next definition */
				map_first_define(map, argv[i + 1]);
				ignore_next_argv = 1;
			} else {
				/* ignore the first 2 characters */
				argv[i] += 2;
				map_first_define(map, argv[i]);
			}
		} else if (strncmp(argv[i], "-o", 2) == 0) {
			if (strlen(argv[i]) == 2) {
				strcpy(*output, argv[i + 1]);
				ignore_next_argv = 1;
			} else {
				argv[i] += 2;
				strcpy(*output, argv[i]);
			}
		} else if (strncmp(argv[i], "-I", 2) == 0) {
			if (strlen(argv[i]) == 2) {
				folders[++pos_folder] = argv[i + 1];
				ignore_next_argv = 1;
			} else {
				argv[i] += 2;
				folders[++pos_folder] = argv[i];
			}
		} else if (*input == NULL) {
			*input = calloc(strlen(argv[i]) + 1, sizeof(char));
			if (*input == NULL)
				exit(12);
			strcpy(*input, argv[i]);
		} else if (*output == NULL) {
			*output = calloc(strlen(argv[i]) + 1, sizeof(char));
			if (*output == NULL)
				exit(12);
			strcpy(*output, argv[i]);
		} else {
			exit(12);
		}
	}
	if (*input == NULL)
		*in = stdin;
	else
		*in = fopen(*input, "r");

	if (*output == NULL)
		*out = stdout;
	else
		*out = fopen(*output, "w");
}

void remove_define(char *line, hashmap *map)
{
	char *symbol = NULL;
	/* erase the first part of the line */
	line += strlen("#undef ");

	symbol = strtok(line, "\n ");

	delete_key(map, symbol);
}

int find_last_comma(char *my_string)
{
	int i = 0, pos = -1;

	for (; my_string[i] != '\0'; i++)
		if (my_string[i] == '\"')
			pos = i;
	return pos;
}

char **list_of_words(char *line)
{
	char **words = NULL;
	char *token = NULL;
	int j = -1;
	int i = -1;
	char *copy_line = calloc((strlen(line) + 1), sizeof(char));

	if (copy_line == NULL)
		exit(12);
	strcpy(copy_line, line);
	words = calloc(100, sizeof(char *));

	if (words == NULL)
		exit(12);

	for (j = 0; j < 100; j++)
		words[j] = NULL;

	token = strtok(copy_line, "\t []{}<>=+-*/%!&|^.,:;().");
	while (token) {
		i++;
		words[i] = calloc(strlen(token) + 1, sizeof(char));
		if (words[i] == NULL)
			exit(12);
		strcpy(words[i], token);
		token = strtok(NULL, "\t []{}<>=+-*/%!&|^.,:;().");
	}
	free(copy_line);
	return words;
}

void map_basic(char *line, hashmap *map)
{
	char **list_words = NULL;
	char *second_part = NULL;
	int i = 0, j = 0;
	int ok = 0;
	int position_key = -1;
	char copy_line[256] = {0};

	for (; i < map->maximum_capacity; i++) {
		if (map->elements[i] != NULL) {
			ok = 0;
			list_words = list_of_words(line);

			for (j = 0; ok == 0 && list_words[j] != NULL; j++) {
				if (strcmp(list_words[j],
					   map->elements[i]->key) == 0) {
					ok = 1;
				}
			}
			for (j = 0; j < 100; j++)
				if (list_words[j] != NULL)
					free(list_words[j]);
			free(list_words);
			if (ok == 1) {
				second_part =
				    strstr(line, map->elements[i]->key);
				position_key = second_part - line;

				strncpy(copy_line, line, position_key);
				second_part += strlen(map->elements[i]->key);
				strcat(copy_line, map->elements[i]->value);
				strcat(copy_line, second_part);
				strcpy(line, copy_line);
				map_basic(line, map);
			}
		}
	}
}

void map_complex(char *line, hashmap *map, int pos_last_comma)
{
	int i = 0;
	char second[256] = {0};
	char copy_line[256] = {0};

	strncpy(copy_line, line, pos_last_comma + 1);
	for (i = pos_last_comma + 1; i < (int) strlen(line); i++)
		second[i - pos_last_comma - 1] = line[i];
	map_basic(second, map);
	strcpy(line, copy_line);
	strcat(line, second);
}

int solve_if_elif(char *line, hashmap *map)
{
	char *pos_value = NULL;

	if (strncmp(line, "#if ", strlen("#if ")) == 0)
		line += strlen("#if ");
	else
		line += strlen("#elif ");
	if (line[strlen(line) - 1] == '\n')
		line[strlen(line) - 1] = '\0';
	pos_value = get_value(map, line);

	if (pos_value) {
		if (strcmp(pos_value, "0") != 0 &&
		    strcmp(pos_value, "FALSE") != 0) {
			return 1;
		} else {
			return 0;
		}
	}
	return strcmp(line, "0") != 0 && strcmp(line, "FALSE") != 0;
}

int solve_ifdef(char *line, hashmap *map)
{
	line += strlen("#ifedf ");
	if (line[strlen(line) - 1] == '\n')
		line[strlen(line) - 1] = '\0';
	return get_value(map, line) == NULL ? 0 : 1;
}

int solve_ifndef(char *line, hashmap *map)
{
	line += strlen("#ifndef ");
	if (line[strlen(line) - 1] == '\n')
		line[strlen(line) - 1] = '\0';
	return get_value(map, line) == NULL ? 1 : 0;
}

void get_path(char **folders, int count_folders, char *path, char *line,
	      char *current_input)
{
	FILE *try_in = NULL;
	int i = 0;

	line += strlen("#include \"");
	line[strlen(line) - 2] = '\0';
	if (strlen(current_input) != 0) {
		char *find_last = strrchr(current_input, '/');
		int last_pos = find_last - current_input;

		strncpy(path, current_input, last_pos + 1);
		strcat(path, line);
		try_in = fopen(path, "r");

		if (try_in != NULL) {
			fclose(try_in);
			return;
		}
	}

	for (; i < count_folders; i++) {
		strcpy(path, folders[i]);
		strcat(path, "/");
		strcat(path, line);
		try_in = fopen(path, "r");

		if (try_in != NULL) {
			fclose(try_in);
			return;
		}
	}

	memset(path, 0, sizeof(path));
}

void add_one_line_define(char *line, hashmap *map)
{
	char *symbol = NULL;
	char *value = NULL;
	char final_value[LINE_LENGTH] = {0};
	int pos_last_comma = -1;

	/* erase the first part of the line */
	line += strlen("#define ");


	symbol = strtok(line, "\n ");
	value = strtok(NULL, "\n ");
	while (value) {
		strcat(final_value, value);
		value = strtok(NULL, "\n ");
		/* if not the last value */
		if (value)
			strcat(final_value, " ");
	}

	pos_last_comma = find_last_comma(final_value);

	if (pos_last_comma < 0)
		map_basic(final_value, map);
	else
		map_complex(final_value, map, pos_last_comma);
	add_pair(map, symbol, final_value);
}

void process_file(hashmap *map, FILE *in, FILE *out,
		  char **folders, int count_folders, char *input_file)
{
	char line[LINE_LENGTH] = {0};
	char composite_define[LINE_LENGTH * 2] = {0};
	int next_line_define = 0;
	int past_condition_true = 1;
	FILE *header_input = NULL;

	while (fgets(line, LINE_LENGTH, in) != NULL) {
		if (strncmp(line, "#endif", 6) == 0) {
			past_condition_true = 1;
			continue;
		} else if (past_condition_true == 0 &&
			   strncmp(line, "#else", 5) != 0 &&
			   strncmp(line, "#elif", 5) != 0) {
			continue;
		}

		if (next_line_define == 1) {
			if (line[strlen(line) - 1] == '\n' &&
			    line[strlen(line) - 2] == 92) {
				/* there is a next line of the definition */
				strncat(composite_define, line,
					strlen(line) - 2);
			} else {
				next_line_define = 0;
				strncat(composite_define, line,
					strlen(line) - 1);
				add_one_line_define(composite_define, map);
				memset(composite_define, 0,
				       sizeof(composite_define));
			}
		} else if (strncmp(line, "#define", strlen("#define")) == 0) {
			if (line[strlen(line) - 1] == '\n' &&
			    line[strlen(line) - 2] == 92) {
				/* there is a next line of the definition */
				strncpy(composite_define, line,
					strlen(line) - 2);
				next_line_define = 1;
			} else {
				add_one_line_define(line, map);
			}
		} else if (strncmp(line, "#undef", strlen("#undef")) == 0) {
			remove_define(line, map);
		} else if (strncmp(line, "#if ", 4) == 0 ||
			   strncmp(line, "#elif", 5) == 0) {
			past_condition_true = solve_if_elif(line, map);
		} else if (strncmp(line, "#else", 5) == 0) {
			past_condition_true = 1 - past_condition_true;
		} else if (strncmp(line, "#ifdef", 6) == 0) {
			past_condition_true = solve_ifdef(line, map);
		} else if (strncmp(line, "#ifndef", 7) == 0) {
			past_condition_true = solve_ifndef(line, map);
		} else if (strncmp(line, "#include", 8) == 0) {
			char path[LINE_LENGTH] = {0};

			get_path(folders, count_folders, path, line,
				 input_file);
			if (strlen(path) == 0)
				exit(12);
			header_input = fopen(path, "r");

			process_file(map, header_input, out, folders,
				     count_folders, input_file);
			fclose(header_input);
		} else {
			int pos_last_comma = find_last_comma(line);

			if (pos_last_comma < 0)
				map_basic(line, map);
			else
				map_complex(line, map, pos_last_comma);
			fprintf(out, "%s", line);
		}
		memset(line, 0, sizeof(char));
	}
}

int main(int argc, char *argv[])
{
	hashmap *map = init_hashmap();
	char *input_file = NULL;
	char *output_file = NULL;
	int count_folders = find_no_directories(argv, argc);
	FILE *in = NULL;
	FILE *out = NULL;
	int past_condition_true = 1;
	int i = 0, j = 0;
	char **folders = calloc(count_folders, sizeof(char *));

	if (folders == NULL)
		exit(12);
	map_parameters(map, argc, argv, &input_file, &output_file, &in, &out,
		       folders);
	if (in == NULL || out == NULL)
		exit(12);
	process_file(map, in, out, folders, count_folders, input_file);
	if (input_file != NULL) {
		free(input_file);
		fclose(in);
	}
	if (output_file != NULL) {
		free(output_file);
		fclose(out);
	}
	delete_map(map);
	free(folders);
	return 0;
}
