#include <stdio.h>
#include <stdlib.h>
#include "List.h"

List *create() {
    List *l = (List *) malloc(sizeof(List));
    l->head = NULL;
    l->tail = NULL;
    l->size = 0;
    return l;
}

void add(unsigned long int elem, List *l) {
    if(!l)
        return;
    
    Node *newNode = (Node *) malloc(sizeof(Node));
    newNode->data = elem;
    newNode->next = NULL;
    
    if(!l->head) {
        l->head = l->tail = newNode;
    } else {
        l->tail->next = newNode;
        l->tail = l->tail->next;
    }
    l->size++;
}

void clearList(List *l) {
    if(!l)
        return;
    
    Node *iterator = l->head;
    Node *aux;
    while(iterator) {
        aux = iterator;
        iterator = iterator->next;
        free(aux);
    }
    
    free(l);
}

void traverse(List *l) {
    if(!l)
        return;
    
    Node *aux = l->head;
    while(aux) {
        printf("Element from list: %lu\n", aux->data);
        aux = aux->next;
    }
}
