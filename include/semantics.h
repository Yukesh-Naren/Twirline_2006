#ifndef SEMANTICS_H
#define SEMANTICS_H

#include<stdio.h>
#include<stdlib.h>
#include "AST.h"
typedef struct Symbol{
    char name[50];
    int is_init;
    int value;
    struct Symbol* next;
}Symbol;

void add_symbol(char* name , int init);
Symbol* lookup_symbol(char* name);
void print_symbol_table();
void check_semantics(Node* node);
#endif