//
//  List.h
//  Encoder
//
//  Created by Henrique de Almeida Machado da Silveira on 10/12/16.
//  Copyright © 2016 HenriSilver. All rights reserved.
//

#ifndef List_h
#define List_h

typedef struct node {
    struct node* next;
    unsigned long int data;
} Node;

typedef struct list {
    Node *head;
    Node *tail;
    unsigned long int size;
} List;

List *create();
void add(unsigned long int elem, List *l);
void clearList(List *l);
void traverse(List *l);

#endif /* List_h */
