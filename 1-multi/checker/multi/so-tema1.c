#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "so-tema1.h"

int insertMapping(Map *map, char **argument) {
	char *symbol = NULL;
	char *mapping = NULL;
	char *token = NULL;
	char delimiters[] = "= ";
	int index = 0;
	int code = 0;

	char *temp = malloc((strlen(*argument) + 1) * sizeof(char));
	if (temp == NULL) {
		return ERROR_ALLOC;
	}

	if (*argument[0] == ' ') {
		(*argument)++;
	}
	strcpy(temp, *argument);

	while (index < 2) {
		if (token == NULL) {
			token = strtok(temp, delimiters);
			symbol = malloc((strlen(token) + 1) * sizeof(char));
			if (symbol == NULL) {
				return ERROR_ALLOC;
			}
			strcpy(symbol, token);
		} else {
			if (*argument[0] == '=') {
				token = strtok(NULL, delimiters);
				mapping = malloc((strlen(token) + 1) * sizeof(char));
				if (mapping == NULL) {
					return ERROR_ALLOC;
				}
				strcpy(mapping, token);
				(*argument)++;
			} else {
				break;
			}
		}
		*argument += strlen(token);
		index++;
	}

	if (mapping == NULL) {
		mapping = "";
	}
	code = put(map, symbol, mapping);
	if (code) {
		return ERROR_ALLOC;
	}
	free(temp);
	free(symbol);
	if (strcmp(mapping, "") != 0) {
		free(mapping);
	}

	return 0;
}

int getFilename(char **argument, char **filename) {
	char *token = NULL;
	char *temp = strdup(*argument);
	if (temp == NULL) {
		return ERROR_ALLOC;
	}

	token = strtok(temp, " ");
	*filename = strdup(token);
	if (*filename == NULL) {
		return ERROR_ALLOC;
	}

	strcpy(*filename, token);
	*argument += strlen(*filename);

	free(temp);
	return 0;
}

int getParameters(int argc, char **argv, char **argumentsString) {
	unsigned long lengthParams = 0;
	int i = 0;

	for (i = 1; i < argc; i++) {
		lengthParams += strlen(argv[i]) + 1;
	}

	*argumentsString = calloc(lengthParams + 1, sizeof(char));
	if (*argumentsString == NULL) {
		return ERROR_ALLOC;
	}

	if (argc > 1) {
		strcpy(*argumentsString, argv[1]);
	}

	for (i = 2; i < argc; i++) {
		strcat(*argumentsString, " ");
		strcat(*argumentsString, argv[i]);
	}

	return 0;
}

int countCharsInSubstring(char *s1, char *s2, char c) {

	return s1 == s2
		   ? 0
		   : countCharsInSubstring(s1 + 1, s2, c) + (*s1 == c);
}

void findIndexOfToken(char *inputBuffer, char **p, char *token) {
	int no_quotes = 0;
	char *auxPointer = inputBuffer;

	do {
		*p = strstr(auxPointer, token);
		no_quotes += countCharsInSubstring(auxPointer, *p, '\"');
		auxPointer = *p + strlen(token);
	} while (no_quotes % 2 != 0);

}

int parseDefine(Map *map, char *token, FILE *streamRead, char *inputBuffer) {
	char *key = NULL;
	char *mapping;
	char *auxToken = NULL;
	char *auxPointer = NULL;
	char *auxPointer2 = NULL;
	char *temp = NULL;
	char *pointerKey = NULL;
	char *auxString;
	int code = 0;

	mapping = malloc(LINE_SIZE * sizeof(char));
	if (mapping == NULL) {
		return ERROR_ALLOC;
	}

	token = strtok(NULL, DELIMITERS);
	key = token;

	token = strtok(NULL, "\\\n");
	if (token == NULL) {
		mapping[0] = '\0';
		code = put(map, key, mapping);
		if (code) {
			return ERROR_ALLOC;
		}
		free(mapping);
		return 0;
	} else {
		pointerKey = strstr(inputBuffer, key);
		pointerKey += strlen(key) + 1;
		if (inputBuffer[strlen(inputBuffer) - 2] == '\\') {
			strncpy(token, pointerKey, strlen(pointerKey) - 2);
		} else {
			strncpy(token, pointerKey, strlen(pointerKey) - 1);
		}
		strcpy(mapping, token);
	}

	auxToken = strtok(token, DELIMITERS);
	while (auxToken != NULL) {
		char *value = search(*map, auxToken);
		if (value != NULL) {
			auxPointer = strstr(mapping, auxToken);
			if (auxPointer != NULL) {
				auxPointer2 = auxPointer + strlen(auxToken);
				auxString = strdup(auxPointer2);
				if (auxString == NULL) {
					return ERROR_ALLOC;
				}
				strcpy(auxPointer, value);
				strcpy(auxPointer + strlen(value), auxString);
				free(auxString);
			}
		}
		auxToken = strtok(NULL, DELIMITERS);
		if (auxToken == NULL && temp != NULL) {
			free(temp);
		}

		while (auxToken == NULL && inputBuffer[strlen(inputBuffer) - 2] == '\\') {
			if (fgets(inputBuffer, LINE_SIZE, streamRead) != NULL) {
				temp = strdup(inputBuffer);
				if (temp == NULL) {
					return ERROR_ALLOC;
				}
				token = strtok(temp, "\\\n");
				if (token != NULL) {
					strcat(mapping, token);
					auxToken = strtok(token, DELIMITERS);
				}
			}
		}
	}
	if (mapping[strlen(mapping) - 1] == '\n') {
		mapping[strlen(mapping) - 1] = '\0';
	}
	code = put(map, key, mapping);
	if (code) {
		return ERROR_ALLOC;
	}

	free(mapping);
	return 0;
}

void evaluateExpression(Map *map, char *token, int *executeCode) {
	char *value = NULL;
	token = strtok(NULL, "\n");
	value = search(*map, token);
	if ((value != NULL && strlen(value) == 1 && value[0] == '0')
		|| (token != NULL && strlen(token) == 1 && token[0] == '0')) {
		*executeCode = 0;
	} else {
		*executeCode = 1;
	}
}

int parseIncludeDirective(Map **map, char *token, Dirs dirs, char *inputFile, int writeToStdin, FILE *fout) {

	int i = 0;
	char *path = NULL;
	char *p1 = inputFile;
	char *p2 = strstr(p1, "/");
	char inputBufferInclude[LINE_SIZE];
	char outputBufferInclude[LINE_SIZE];
	int executeCode = 1;
	FILE *fp;

	outputBufferInclude[0] = '\0';


	while (p2 != NULL) {
		p1 += (p2 - p1);
		p2 = strstr(p1 + 1, "/");
	}
	token = strtok(NULL, "\n");
	token[strlen(token) - 1] = '\0';
	token++;

	path = calloc(p1 - inputFile + strlen(token) + 2, sizeof(char));
	if (path == NULL) {
		return ERROR_ALLOC;
	}
	strncpy(path, inputFile, p1 - inputFile);
	strcat(path, "/");
	strcat(path, token);

	fp = fopen(path, "r");

	if (fp == NULL) {
		for (i = 0; i < dirs.nrDirs; i++) {
			path = realloc(path, (strlen(token) + strlen(dirs.dirPaths[i]) + 2) * sizeof(char));
			if (path == NULL) {
				return ERROR_ALLOC;
			}
			strcpy(path, dirs.dirPaths[i]);
			strcat(path, "/");
			strcat(path, token);
			fp = fopen(path, "r");

			if (fp != NULL) {
				break;
			}
		}
	}

	DIE(fp == NULL, "Bad include");
	free(path);

	while (fgets(inputBufferInclude, LINE_SIZE, fp) != NULL) {
		int code = parseInputLine(*map, inputBufferInclude, outputBufferInclude, fp, dirs, &executeCode, inputFile,
								  writeToStdin, fout);
		if (code) {
			return ERROR_ALLOC;
		} else {
			if (writeToStdin) {
				printf("%s", outputBufferInclude);
			} else {
				fprintf(fout, "%s", outputBufferInclude);
			}
		}
		outputBufferInclude[0] = '\0';
	}

	fclose(fp);

	return 0;
}

int parseInputLine(Map *map, char *inputBuffer, char *outputBuffer, FILE *streamRead, Dirs dirs, int *executeCode,
				   char *inputFile, int writeToStdin, FILE *fout) {

	char *token = NULL;
	int code = 0;
	char *value = NULL;
	int writeOutput = 1;
	char *temp = strdup(inputBuffer);
	if (temp == NULL) {
		return ERROR_ALLOC;
	}

	token = strtok(temp, DELIMITERS);

	while (token != NULL) {
		writeOutput = 0;
		if (strncmp(token, "#define", strlen("#define")) == 0 && *executeCode == 1) {
			code = parseDefine(map, token, streamRead, inputBuffer);
			if (code) {
				return ERROR_ALLOC;
			}
			break;
		} else if (strcmp(token, "#undef") == 0 && *executeCode == 1) {
			token = strtok(NULL, "\n");
			if (search(*map, token) != NULL) {
				code = deleteEntry(&map, token);
				if (code) {
					return ERROR_ALLOC;
				}
			}
			break;
		} else if ((strcmp(token, "#if") == 0 && *executeCode == 1)
				   || (strcmp(token, "#elif") == 0 && *executeCode == 0)) {
			evaluateExpression(map, token, executeCode);
			break;
		} else if (strncmp(token, "#endif", strlen("#endif")) == 0) {
			*executeCode = 1;
			break;
		} else if (strncmp(token, "#else", strlen("#else")) == 0) {
			*executeCode = (*executeCode == 1) ? 0 : 1;
			break;
		} else if (strcmp(token, "#ifdef") == 0 && *executeCode == 1) {
			token = strtok(NULL, "\n");
			value = search(*map, token);
			*executeCode = value != NULL ? 1 : 0;
			break;
		} else if (strcmp(token, "#ifndef") == 0 && *executeCode == 1) {
			token = strtok(NULL, "\n");
			value = search(*map, token);
			*executeCode = value != NULL ? 0 : 1;
			break;
		} else if (strcmp(token, "#include") == 0 && *executeCode == 1) {
			code = parseIncludeDirective(&map, token, dirs, inputFile, writeToStdin, fout);
			if (code) {
				return ERROR_ALLOC;
			}
			break;
		} else if (*executeCode == 1) {
			writeOutput = 1;
			value = search(*map, token);

			if (value != NULL) {
				char *p = NULL;

				findIndexOfToken(inputBuffer, &p, token);
				strncat(outputBuffer, inputBuffer, p - inputBuffer);
				strncat(outputBuffer, value, strlen(value));
				inputBuffer += (p - inputBuffer + strlen(token));
			}
			token = strtok(NULL, DELIMITERS);
		} else {
			break;
		}
	}

	if (writeOutput == 1) {
		strncat(outputBuffer, inputBuffer, strlen(inputBuffer));
	}

	free(temp);
	return 0;
}

int execute(Map *map, char *inputFile, const char *outputFile, Dirs dirs) {
	char inputBuffer[LINE_SIZE];
	char outputBuffer[LINE_SIZE];
	int executeCode = 1;

	FILE *fin = NULL;
	FILE *fout = NULL;
	FILE *streamRead = NULL;

	int writeToStdin = 0;

	outputBuffer[0] = '\0';


	if (inputFile == NULL) {
		streamRead = stdin;
	} else {
		fin = fopen(inputFile, "r");
		DIE(fin == NULL, "No input file found!");
		streamRead = fin;
	}

	if (outputFile == NULL) {
		writeToStdin = 1;
	} else {
		fout = fopen(outputFile, "w+");
		DIE(fout == NULL, "Error opening output file!");
	}

	while (fgets(inputBuffer, LINE_SIZE, streamRead) != NULL) {
		int code = parseInputLine(map, inputBuffer, outputBuffer, streamRead, dirs, &executeCode, inputFile,
								  writeToStdin, fout);
		if (code) {
			return ERROR_ALLOC;
		} else {
			if (writeToStdin) {
				printf("%s", outputBuffer);
			} else {
				fprintf(fout, "%s", outputBuffer);
			}
		}
		outputBuffer[0] = '\0';
	}

	if (fout != NULL) {
		fclose(fout);
	}
	if (fin != NULL) {
		fclose(fin);
	}

	return 0;
}

int parseParameters(char *argumentsString) {
	Dirs dirs;

	char *outputFile = NULL;
	char *inputFile = NULL;
	int code = 0;
	int i = 0;
	Map map;

	dirs.dirPaths = NULL;
	dirs.capacity = 1;
	dirs.nrDirs = 0;

	map.buckets = calloc(MAP_SIZE, sizeof(Bucket *));
	if (map.buckets == NULL) {
		return ERROR_ALLOC;
	}

	if (argumentsString != NULL) {
		while (*argumentsString) {
			if (argumentsString[0] == '-') {
				argumentsString++;
				switch (argumentsString[0]) {
					case 'o':
						argumentsString++;
						if (outputFile == NULL) {
							if (argumentsString[0] == ' ') {
								argumentsString++;
							}
							code = getFilename(&argumentsString, &outputFile);
							if (code) {
								return ERROR_ALLOC;
							}
						} else {
							perror("Too many output files!");
							exit(1);
						}
						break;
					case 'D':
						argumentsString++;
						code = insertMapping(&map, &argumentsString);
						if (code) {
							return ERROR_ALLOC;
						}
						break;
					case 'I':
						if (dirs.dirPaths == NULL) {
							dirs.dirPaths = calloc((size_t) dirs.capacity, sizeof(char *));
							if (dirs.dirPaths == NULL) {
								return ERROR_ALLOC;
							}
						}
						if (dirs.capacity <= dirs.nrDirs) {
							dirs.capacity *= 2;
							dirs.dirPaths = realloc(dirs.dirPaths, dirs.capacity * sizeof(char *));
							if (dirs.dirPaths == NULL) {
								return ERROR_ALLOC;
							}
						}
						argumentsString++;
						if (argumentsString[0] == ' ') {
							argumentsString++;
						}

						code = getFilename(&argumentsString, &dirs.dirPaths[dirs.nrDirs]);
						if (code) {
							return ERROR_ALLOC;
						}
						dirs.nrDirs++;
						break;
					default:
						perror("Bad parameter!");
						exit(1);
				}
			} else if (argumentsString[0] != ' ') {
				if (inputFile == NULL) {
					code = getFilename(&argumentsString, &inputFile);
					if (code) {
						return ERROR_ALLOC;
					}
				} else {
					if (outputFile != NULL) {
						perror("Too many input files!");
						exit(1);
					} else {
						code = getFilename(&argumentsString, &outputFile);
						if (code) {
							return ERROR_ALLOC;
						}
					}
				}
			}
			argumentsString++;
		}
	}

	code = execute(&map, inputFile, outputFile, dirs);
	if (code) {
		return ERROR_ALLOC;
	}
	if (outputFile != NULL) {
		free(outputFile);
	}
	if (inputFile != NULL) {
		free(inputFile);
	}
	if (dirs.dirPaths != NULL) {
		for (i = 0; i < dirs.nrDirs; i++) {
			free(dirs.dirPaths[i]);
		}
		free(dirs.dirPaths);
	}
	freeHashMap(&map);

	return 0;
}

int main(int argc, char *argv[]) {
	char *argumentsString = NULL;
	int code = 0;

	if (argc > 1) {
		code = getParameters(argc, argv, &argumentsString);
		if (code) {
			return ERROR_ALLOC;
		}
	}

	code = parseParameters(argumentsString);
	if (code) {
		return ERROR_ALLOC;
	}

	free(argumentsString);

	return 0;
}