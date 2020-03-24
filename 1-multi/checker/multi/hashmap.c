#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "so-tema1.h"

int isEmpty(Bucket *head) {
	return head == NULL;
}

void freeBucket(Bucket **head) {
	Bucket *p;

	if (*head == NULL) {
		return;
	}

	p = *head;
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
	Bucket *aux;
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

	aux = map->buckets[hashIndex];

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

int deleteEntry(Map **map, char *key) {
	int hashIndex = hashcode(key);
	Bucket *aux = (*map)->buckets[hashIndex];
	Bucket *p;

	if (strcmp(aux->key, key) == 0) {
		(*map)->buckets[hashIndex] = (*map)->buckets[hashIndex]->next;
		free(aux->key);
		free(aux->value);
		free(aux);
		return 0;
	}

	while (aux->next != NULL) {
		if (strcmp(aux->next->key, key) == 0) {
			p = aux->next;
			aux->next = aux->next->next;
			free(aux->key);
			free(aux->value);
			free(p);
			return 0;
		}
		aux = aux->next;
	}

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