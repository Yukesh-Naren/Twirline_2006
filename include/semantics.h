// #ifndef SEMANTICS_H
// #define SEMANTICS_H

// #include<stdio.h>
// #include<stdlib.h>
// #include "AST.h"
// // typedef struct Symbol{
// //     char name[50];
// //     int is_init;
// //     int value;
// //     struct Symbol* next;
// // }Symbol;

// // void add_symbol(char* name , int init);
// // Symbol* lookup_symbol(char* name);
// // void print_symbol_table();
// // void check_semantics(Node* node);

// char* analyze(Node* root);

// void semanticError(const char *msg);
// #endif
#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdio.h>
#include <stdlib.h>
#include "AST.h"

// 1. Definition of the Symbol structure
typedef struct Symbol {
    char name[50];
    int is_init;
    int value;           // Assuming you might store values later
    struct Symbol* next;
} Symbol;

// 2. Prototypes for functions in semantics.c
void check_semantics(Node* root);
char* evaluate_expression(Node* root);
char* getLeafType(Node* node);
char* checkOpType(char* left, char* right, char* op);
void semanticError(const char *msg);

// 3. Prototypes for Symbol Table functions (likely in symbol_table.c or similar)
void add_symbol(char* name, int init);
Symbol* lookup_symbol(char* name);
void print_symbol_table();

#endif