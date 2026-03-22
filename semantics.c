#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantics.h"

/* Head of symbol table */
Symbol* head = NULL;

/* ---------------- ERROR HANDLER ---------------- */

void semanticError(const char *msg)
{
    printf("Semantic Error: %s\n", msg);
    exit(1);
}

/* ---------------- SYMBOL TABLE ---------------- */

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

void add_symbol(char* name, int init)
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
    newSym->value = 0;
    newSym->next = head;

    head = newSym;
}

/* ---------------- PRINT SYMBOL TABLE ---------------- */

void print_symbol_table()
{
    printf("\n===== SYMBOL TABLE =====\n");
    printf("%-10s %-10s %-10s\n", "NAME", "INIT", "VALUE");
    printf("--------------------------------\n");

    Symbol* temp = head;

    while (temp != NULL)
    {
        printf("%-10s %-10d %-10d\n",
               temp->name,
               temp->is_init,
               temp->value);
        temp = temp->next;
    }

    printf("================================\n");
}

/* ---------------- EXPRESSION EVALUATION ---------------- */

int evaluate_expression(Node* root)
{
    if (root == NULL)
        semanticError("Invalid expression");

    if (root->type == NODE_CONST)
    {
        return atoi(root->val);
    }

    if (root->type == NODE_ID)
    {
        Symbol* sym = lookup_symbol(root->val);

        if (sym == NULL)
        {
            char msg[100];
            sprintf(msg, "Variable '%s' not declared", root->val);
            semanticError(msg);
        }

        if (!sym->is_init)
        {
            char msg[100];
            sprintf(msg, "Variable '%s' used without initialization", root->val);
            semanticError(msg);
        }

        return sym->value;
    }

    if (root->type == NODE_OP)
    {
        if(strcmp(root->val, "!") == 0){
            int val = evaluate_expression(root->left);
            return !val;
    }
        int leftVal = evaluate_expression(root->left);
        int rightVal = evaluate_expression(root->right);

        if (strcmp(root->val, "+") == 0)
            return leftVal + rightVal;

        else if (strcmp(root->val, "-") == 0)
            return leftVal - rightVal;

        else if (strcmp(root->val, "*") == 0)
            return leftVal * rightVal;

        else if (strcmp(root->val, "/") == 0)
        {
            if (rightVal == 0)
                semanticError("Division by zero");
            return leftVal / rightVal;
        }

        else if (strcmp(root->val, "%") == 0)
        {
            if (rightVal == 0)
                semanticError("Modulo by zero");
            return leftVal % rightVal;
        }

        else if (strcmp(root->val, "<=") == 0)
        return leftVal <= rightVal;

        else if (strcmp(root->val, ">=") == 0)
        return leftVal >= rightVal;

        else if (strcmp(root->val, "<") == 0)
        return leftVal < rightVal;

        else if (strcmp(root->val, ">") == 0)
        return leftVal > rightVal;

        else if (strcmp(root->val, "==") == 0)
        return leftVal == rightVal;

        else if (strcmp(root->val, "!=") == 0)
        return leftVal != rightVal;

        else if (strcmp(root->val, "&&") == 0)
        return leftVal && rightVal;

        else if (strcmp(root->val, "||") == 0)
        return leftVal || rightVal;

        else
        {
            semanticError("Unknown operator");
        }
    }

    if (root->type == NODE_ASSIGN)
    {
        if (root->left == NULL || root->left->type != NODE_ID)
            semanticError("Invalid assignment");

        Symbol* sym = lookup_symbol(root->left->val);

        if (sym == NULL)
        {
            char msg[100];
            snprintf(msg,sizeof(msg), "Variable '%s' not declared", root->left->val);
            semanticError(msg);
        }

        int value = evaluate_expression(root->right);

        sym->value = value;
        sym->is_init = 1;

        return value;
    }

    semanticError("Unsupported node type");
    return 0;
}

/* ---------------- MAIN SEMANTIC CHECK ---------------- */

void check_semantics(Node* root)
{
    Node* current = root;
    while (current != NULL)
    {
        if (current->type == NODE_DECL)
        {
            /* int a; */
            add_symbol(current->left->val, 0);
        }
        else if (current->type == NODE_DECL_ASSN)
        {
            /* int a = expr; */

            add_symbol(current->left->left->val, 0);

            Symbol* sym = lookup_symbol(current->left->left->val);

            int value = evaluate_expression(current->left->right);

            sym->value = value;
            sym->is_init = 1;
        }
        else if (current->type == NODE_ASSIGN)
        {
            evaluate_expression(current);
        }

        else if (current->type == NODE_IF)
        check_if(current);
        
        else if (current->type == NODE_WHILE)
        check_while(current);

        else if (current->type == NODE_PRINT)
        check_print(current);

        else if(current->type == NODE_INPUT)
        check_input(current);

        else
        {
            evaluate_expression(current);
        }

        current = current->next;
    }

}

void check_print(Node* node)
{
    if(node == NULL) {
        semanticError("Invalid print statement");
        return;
    }

    Node* arg = node->left;

    if (arg == NULL){
        semanticError("print has no arguments");
        return;
    }
    while(arg != NULL){
        if(arg->type != NODE_STRING)
        evaluate_expression(arg);
        arg = arg->next;
    }
}
void check_if(Node* node)
{
    
    int cond = evaluate_expression(node->left);

    if(node->right == NULL) return;

    if(node->right->type == NODE_ELSE)
    {
        Node* elseWrap = node->right;

        if(cond)
        {
            check_semantics(elseWrap->left);
        }
        else
        {
            if(elseWrap->right != NULL)
            {
                if(elseWrap->right->type == NODE_IF)
                    check_if(elseWrap->right);
                else
                    check_semantics(elseWrap->right);
            }
        }
    }
    else
    {
        if(cond)
            check_semantics(node->right);
    }
}

void check_while(Node* node)
{
    if (node == NULL) return;
    if (node ->left == NULL){
        semanticError("while condition missing!");
        return;
    }
    while(evaluate_expression(node->left)){
        check_semantics(node->right);
    }
}

void check_input(Node* node)
{
    if(node == NULL || node->left == NULL || node->left->type != NODE_ID)
    semanticError("Invalid input statement");

    Symbol* sym = lookup_symbol(node->left->val);
    if(sym == NULL){
        char msg[100];
        sprintf(msg, "Variable '%s' not declared", node->left->val);
        semanticError(msg);
    }
    sym->is_init = 1;
}