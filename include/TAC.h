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
} TAC;

extern TAC* tacHead;
extern TAC* tacTail;
extern int tempcount;
extern int labelcount;

char* newTemp();
char* new_label();

TAC* CreateTAC(char* result, char* arg1, char* op, char* arg2, int type);
void appendTAC(TAC* node);

void emit_if_goto(char* cond, char* label);
void emit_goto(char* label);
void emit_label(char* label);

char* get_array_base_name(Node* access);
char* generate_array_offset(Node* access);

int get_expr_type(Node* root);
char* generate_expr(Node* root);

void generate_assign(Node* node);
void generate_array_assign(Node* node);
void generate_if(Node* node);
void generate_while(Node* node);
void generate_print(Node* node);
void generate_input(Node* node);

void generate_stmt(Node* node);
void generate_stmt_list(Node* node);
void generate_TAC(Node* root);
void print_TAC();
void generate_function_def(Node* node);
void generate_return(Node* node);

#endif