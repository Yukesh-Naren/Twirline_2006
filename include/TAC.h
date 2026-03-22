#ifndef TAC_H
#define TAC_H

#include "ast.h"

typedef struct TAC {
    char result[20];
    char arg1[20];
    char op[10];
    char arg2[20];
    int type;
    struct TAC* next;
}TAC;

extern TAC* tacHead;
extern TAC* tacTail;
extern int tempcount;

char* newTemp();
char* new_label();

void emit_if_goto(char* cond, char* label);
void emit_goto(char* label);
void emit_label(char* label);
int get_expr_tpe(char* root);
char* generate_expr(Node* root);
void generate_assign(Node* node);
void generate_if(Node* node);
void generate_while(Node* node);
void generate_print(Node* node);
void generate_stmt(Node* node);
void generate_stmt_list(Node* node);
void generate_TAC(Node* root);
void generate_input(Node* root);
void generate_array_assign(Node* node);
void print_TAC();

#endif