
#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdio.h>
#include <stdlib.h>
#include "AST.h"

#define TYPE_INT 1
#define TYPE_FLOAT 2
#define TYPE_CHAR 3

typedef struct Symbol {
    char name[50];
    int is_init;
    float value; 
    int type;         
    struct Symbol* next;
} Symbol;


void check_semantics(Node* root);
float evaluate_expression(Node* root);
int get_symbol_type(const char* name);
char* checkOpType(char* left, char* right, char* op);
void semanticError(const char *msg);
void check_if(Node* node);
void check_while(Node* node);
void check_print(Node* node);
void check_input(Node* node);
void add_symbol(char* name, int init , int type);
Symbol* lookup_symbol(char* name);
void print_symbol_table();

#endif