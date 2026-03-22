#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantics.h"

Symbol* head = NULL;

void semanticError(const char *msg)
{
    printf("Semantic Error: %s\n", msg);
    exit(1);
}

int get_symbol_type(const char *name) {
    if(name == NULL) return TYPE_INT;

    if(strncmp(name, "tmp", 3) == 0)
        return TYPE_INT;

    Symbol* sym = lookup_symbol((char*)name);

    if (sym == NULL) {
        char msg[100];
        sprintf(msg, "Variable '%s' not declared (type lookup)", name);
        semanticError(msg);
    }

    return sym->type;
}

Symbol* lookup_symbol(char* name)
{
    Symbol* temp = head;

    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }

    return NULL;
}

void add_symbol(char* name, int init, int type)
{
    if (lookup_symbol(name) != NULL)
    {
        char msg[100];
        sprintf(msg, "Variable '%s' already declared", name);
        semanticError(msg);
    }

    Symbol* newSym = (Symbol*)malloc(sizeof(Symbol));

    strcpy(newSym->name, name);
    newSym->is_init = init;
    newSym->type = type;
    newSym->value = 0.0;

    newSym->is_array = 0;
    newSym->array_size = 0;
    newSym->array_values = NULL;
    newSym->array_init = NULL;

    newSym->next = head;
    head = newSym;
}

void add_array_symbol(char* name, int type, int size)
{
    if (lookup_symbol(name) != NULL)
    {
        char msg[100];
        sprintf(msg, "Variable '%s' already declared", name);
        semanticError(msg);
    }

    if (size <= 0)
        semanticError("Invalid array size");

    Symbol* newSym = (Symbol*)malloc(sizeof(Symbol));

    strcpy(newSym->name, name);
    newSym->is_init = 1;
    newSym->type = type;
    newSym->value = 0.0;

    newSym->is_array = 1;
    newSym->array_size = size;
    newSym->array_values = (float*)calloc(size, sizeof(float));
    newSym->array_init = (int*)calloc(size, sizeof(int));

    newSym->next = head;
    head = newSym;
}

void print_symbol_table()
{
    printf("\n===== SYMBOL TABLE =====\n");
    printf("%-10s %-10s %-10s %-10s %-10s\n", "NAME", "INIT", "VALUE", "ARRAY", "SIZE");
    printf("------------------------------------------------------------\n");

    Symbol* temp = head;

    while (temp != NULL)
    {
        printf("%-10s %-10d %-10.2f %-10d %-10d\n",
               temp->name,
               temp->is_init,
               temp->value,
               temp->is_array,
               temp->array_size);
        temp = temp->next;
    }

    printf("============================================================\n");
}

int get_node_type_from_decl(Node* typeNode)
{
    if (strcmp(typeNode->val, "int") == 0) return TYPE_INT;
    if (strcmp(typeNode->val, "float") == 0) return TYPE_FLOAT;
    if (strcmp(typeNode->val, "char") == 0) return TYPE_CHAR;
    semanticError("Unknown declaration type");
    return TYPE_INT;
}

void validate_assignment_type(Symbol* sym, float value)
{
    if (sym->type == TYPE_INT && value != (int)value) {
        semanticError("Cannot assign float to int variable");
    }

    if (sym->type == TYPE_CHAR) {
        if (value != (int)value || value < 0 || value > 255)
            semanticError("Invalid char assignment");
    }
}

float evaluate_expression(Node* node)
{
    if (node == NULL) return 0;

    if (node->type == NODE_CONST)
        return atof(node->val);

    if (node->type == NODE_ID)
    {
        Symbol* sym = lookup_symbol(node->val);
        if (sym == NULL)
            semanticError("Undeclared variable");

        return sym->value;
    }

    if (node->type == NODE_ARRAY_ACCESS)
    {
        Symbol* sym = lookup_symbol(node->left->val);
        if (!sym)
            semanticError("Undeclared array");

        if (!sym->is_array)
            semanticError("Variable is not an array");

        int idx = (int)evaluate_expression(node->right);

        if (idx < 0 || idx >= sym->array_size)
            semanticError("Array index out of bounds");

        return sym->array_values[idx];
    }

    if (node->type == NODE_OP)
    {
        float left = evaluate_expression(node->left);
        float right = evaluate_expression(node->right);

        if (!strcmp(node->val, "+")) return left + right;
        if (!strcmp(node->val, "-")) return left - right;
        if (!strcmp(node->val, "*")) return left * right;
        if (!strcmp(node->val, "/")) return left / right;
    }

    return 0;
}

void check_semantics(Node* root)
{
    Node* current = root;

    while (current != NULL)
    {
        if (current->type == NODE_DECL)
        {
            char* name = current->left->val;
            int type = get_node_type_from_decl(current->left->left);
            add_symbol(name, 0, type);
        }
        else if (current->type == NODE_ARRAY_DECL)
        {
            char* name = current->left->val;
            int type = get_node_type_from_decl(current->left->left);
            int size = atoi(current->right->val);

            add_array_symbol(name, type, size);
        }
        else if (current->type == NODE_ASSIGN)
        {
            Symbol* sym = lookup_symbol(current->left->val);
            if (!sym)
                semanticError("Undeclared variable");

            float val = evaluate_expression(current->right);
            validate_assignment_type(sym, val);

            sym->value = val;
            sym->is_init = 1;
        }
        else if (current->type == NODE_ARRAY_ASSIGN)
        {
            Node* access = current->left;

            Symbol* sym = lookup_symbol(access->left->val);
            if (!sym)
                semanticError("Undeclared array");

            if (!sym->is_array)
                semanticError("Variable is not an array");

            int idx = (int)evaluate_expression(access->right);

            if (idx < 0 || idx >= sym->array_size)
                semanticError("Array index out of bounds");

            float val = evaluate_expression(current->right);
            validate_assignment_type(sym, val);

            sym->array_values[idx] = val;
            sym->array_init[idx] = 1;
        }
        else
        {
            evaluate_expression(current);
        }

        current = current->next;
    }
}