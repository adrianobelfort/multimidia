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
