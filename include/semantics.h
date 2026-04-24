#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "AST.h"

#define TYPE_INT   1
#define TYPE_FLOAT 2
#define TYPE_CHAR  3

typedef struct Symbol {
    char name[64];
    int is_init;
    int type;
    float value;

    int is_array;
    int dimensions;
    int* dim_sizes;
    float* array_values;
    int* array_init;

    int scope_depth;

    struct Symbol* next;
} Symbol;

typedef struct FunctionSymbol {
    char name[64];
    int param_count;
    int return_type;
    Node* params;

    struct FunctionSymbol* next;
} FunctionSymbol;

extern Symbol* head;
extern FunctionSymbol* functionHead;

void semanticError(const char* msg);

void enter_scope();
void exit_scope();

Symbol* lookup_symbol(char* name);
Symbol* lookup_symbol_current_scope(char* name);

void add_symbol(char* name, int init, int type);
void add_symbol_scoped(char* name, int init, int type, int scope_depth);

void add_array_symbol_nd(char* name, int type, Node* dims);

FunctionSymbol* lookup_function(char* name);
void add_function(char* name, Node* params, int return_type);

int get_node_type_from_decl(Node* typeNode);
void validate_assignment_type(Symbol* sym, float value);

int compute_offset(Symbol* sym, Node* access);
float evaluate_expression(Node* node);

void check_semantics(Node* root);
void print_symbol_table();
#endif