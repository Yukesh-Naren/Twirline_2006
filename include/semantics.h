
#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdio.h>
#include <stdlib.h>
#include "AST.h"


typedef struct Symbol {
    char name[50];
    int is_init;
    int value;          
    struct Symbol* next;
} Symbol;


void check_semantics(Node* root);
int evaluate_expression(Node* root);
char* getLeafType(Node* node);
char* checkOpType(char* left, char* right, char* op);
void semanticError(const char *msg);
void check_if(Node* node);
void check_while(Node* node);

void add_symbol(char* name, int init);
Symbol* lookup_symbol(char* name);
void print_symbol_table();

#endif