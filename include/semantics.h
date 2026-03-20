#ifndef SEMANTICS_H
#define SEMANTICS_H

#include<stdio.h>
#include<stdlib.h>

typedef struct Symbol{
    char name[50];
    int is_init;
    struct Symbol* next;
}Symbol;

void add_symbol(char* name , int init);
int lookup_symbol(char* name);
void print_symbol_table();

#endif