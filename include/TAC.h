#ifndef TAC_H
#define TAC_H

#include "ast.h"

typedef struct TAC {
    char result[20];
    char arg1[20];
    char op[10];
    char arg2[20];
    struct TAC* next;
}TAC;

extern TAC* tacHead;
extern TAC* tacTail;
extern int tempcount;
TAC* CreateTAC( char* result, char* arg1, char* op, char* arg2);
void appendTAC(TAC* node);
char* newTemp();
char* generateTAC(Node* root);
void print_TAC();

#endif