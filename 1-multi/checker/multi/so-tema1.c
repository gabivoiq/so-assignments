#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

#define MAP_SIZE 1000
#define LINE_SIZE 256
#define DELIMITERS "\t []{}<>=+-*/%!&|^.,:;()\\"
#define ERROR_ALLOC 12

typedef struct Bucket {
    char *key;
    char *value;
    struct Bucket *next;
} Bucket;

typedef struct Map {
    struct Bucket **buckets;
} Map;

int isEmpty(Bucket *head) {
    return head == NULL;
}

void freeBucket(Bucket **head) {
    if (*head == NULL) {
        return;
    }

    Bucket *p = *head;
    *head = (*head)->next;
    free(p->key);
    free(p->value);
    free(p);
    freeBucket(head);
}

int hashcode(char *key) {
    int hash = 0;
    while (*key) {
        hash += *key++;
    }
    return hash % MAP_SIZE;

}

int put(Map *map, char *key, char *value) {

    int hashIndex = hashcode(key);

    Bucket *new = malloc(sizeof(Bucket));
    if (new == NULL) {
        return ERROR_ALLOC;
    }
    new->key = malloc((strlen(key) + 1) * sizeof(char));
    if (new->key == NULL) {
        return ERROR_ALLOC;
    }
    new->value = malloc((strlen(value) + 1) * sizeof(char));
    if (new->value == NULL) {
        return ERROR_ALLOC;
    }
    strcpy(new->key, key);
    strcpy(new->value, value);
    new->next = NULL;

    if (isEmpty(map->buckets[hashIndex])) {
        map->buckets[hashIndex] = new;
        return 0;
    }

    Bucket *aux = map->buckets[hashIndex];

    while (aux->next != NULL) {
        if (strcmp(aux->value, value) == 0) {
            return 0;
        }
        aux = aux->next;
    }

    aux->next = new;
    new->next = NULL;

    return 0;
}

char *search(Map map, char *key) {
    int hashIndex = hashcode(key);

    Bucket *auxBucket = map.buckets[hashIndex];

    while (auxBucket != NULL) {
        if (strcmp(auxBucket->key, key) == 0) {
            return auxBucket->value;
        }
        auxBucket = auxBucket->next;
    }
    return NULL;
}

void freeHashMap(Map *map) {
    int i = 0;

    for (i = 0; i < MAP_SIZE; i++) {
        freeBucket(&map->buckets[i]);
    }
    free(map->buckets);
}

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
    char delimiters[] = " ";
    char *token = NULL;
    char *temp = malloc((strlen(*argument) + 1) * sizeof(char));
    if (temp == NULL) {
        return ERROR_ALLOC;
    }
    strcpy(temp, *argument);

    token = strtok(temp, delimiters);
    *filename = malloc((strlen(token) + 1) * sizeof(char));
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
        strncat(*argumentsString, " ", 1);
        strncat(*argumentsString, argv[i], strlen(argv[i]));
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

int parseDefine(Map *map, char *token, FILE* streamRead) {
    char* key = NULL;
    char* mapping = NULL;
    char* auxToken = NULL;
    char* auxPointer = NULL;
    char* auxPointer2 = NULL;
    int code = 0;

    token = strtok(NULL, DELIMITERS);
    key = token;

    token = strtok(NULL, "\n");
    mapping = strdup(token);

    if(mapping == NULL) {
        return ERROR_ALLOC;
    }

    auxToken = strtok(token, DELIMITERS);
    while(auxToken != NULL) {
        char* value = search(*map, auxToken);
        if(value != NULL) {
            auxPointer = strstr(mapping, auxToken);
            if(auxPointer != NULL) {
                auxPointer2 = auxPointer + strlen(auxToken);
                char *auxString = strdup(auxPointer2);
                strcpy(auxPointer, value);
                strcpy(auxPointer+strlen(value), auxString);
                free(auxString);
            }
        }
        auxToken = strtok(NULL, DELIMITERS);
    }
    code = put(map, key, mapping);
    if(code) {
        return ERROR_ALLOC;
    }

    free(mapping);
    return 0;
}


int parseInputLine(Map *map, char *inputBuffer, char *outputBuffer, FILE* streamRead) {
    char *token = NULL;
    int code = 0;
    char *value = NULL;
    char *temp = malloc((strlen(inputBuffer) + 1) * sizeof(char));
    int emptyBuffer = 1;

    if (temp == NULL) {
        return ERROR_ALLOC;
    }

    strcpy(temp, inputBuffer);
    token = strtok(temp, DELIMITERS);

    while (token != NULL) {
        if (strcmp(token, "#define") == 0) {
//            parseDefine(map, token, streamRead);
//            emptyBuffer = 0;
//            char *key = NULL;
//            char *mapping = NULL;
//
//            token = strtok(NULL, DELIMITERS);
//            key = token;
//            token = strtok(NULL, "\n");
//            mapping = token;
//            code = put(map, key, mapping);
//            if (code) {
//                return ERROR_ALLOC;
//            }
            code = parseDefine(map, token, streamRead);
            if(code) {
                return ERROR_ALLOC;
            }
            free(temp);
            strcpy(outputBuffer, "\0");
            return 0;
        }
        value = search(*map, token);

        if (value != NULL) {
            char *p;

            findIndexOfToken(inputBuffer, &p, token);
            strncat(outputBuffer, inputBuffer, p - inputBuffer);
            strncat(outputBuffer, value, strlen(value));
            inputBuffer += (p - inputBuffer + strlen(token));
        }
        token = strtok(NULL, DELIMITERS);
    }

    strncat(outputBuffer, inputBuffer, strlen(inputBuffer));

    free(temp);

    return 0;
}

int execute(Map *map, const char *inputFile, const char *outputFile) {
    char inputBuffer[LINE_SIZE];
    char outputBuffer[LINE_SIZE];
    outputBuffer[0] = '\0';

    FILE *fin = NULL;
    FILE *fout = NULL;
    FILE *streamRead = NULL;

    int writeToStdin = 0;

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
        int code = parseInputLine(map, inputBuffer, outputBuffer, streamRead);
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
    char *outputFile = NULL;
    char *inputFile = NULL;
    char *dir = NULL;
    int code = 0;
    Map map;

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
//                            printf("%s", outputFile);
                        } else {
                            free(inputFile);
                            free(outputFile);
                            perror("Too many output files!");
                            exit(1);
                        }
                        break;
                    case 'D':
                        argumentsString++;
                        insertMapping(&map, &argumentsString);
                        break;
                    case 'I':
                        argumentsString++;
                        if (argumentsString[0] == ' ') {
                            argumentsString++;
                        }
                        if (dir != NULL) {
                            free(dir);
                        }
                        code = getFilename(&argumentsString, &dir);
                        if (code) {
                            return ERROR_ALLOC;
                        }
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
//                printf("%s", inputFile);
                } else {
                    if (outputFile != NULL) {
                        free(inputFile);
                        free(outputFile);
                        perror("Too many input files!");
                        exit(1);
                    } else {
                        code = getFilename(&argumentsString, &outputFile);
                        if (code) {
                            return ERROR_ALLOC;
                        }
//                    printf("%s", outputFile);
                    }
                }
            }
            argumentsString++;
        }
    }

    code = execute(&map, inputFile, outputFile);
    if (code) {
        return ERROR_ALLOC;
    }
    if (outputFile != NULL) {
        free(outputFile);
    }
    if (inputFile != NULL) {
        free(inputFile);
    }
    if (dir != NULL) {
        free(dir);
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