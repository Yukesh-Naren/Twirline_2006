#include "semantics.h"
#include "parser.h"
#include <string.h>
#include<stdio.h>
#include<stdlib.h>

Symbol* head = NULL;

void add_symbol(char* name , int init){
    Symbol* s = head;
    while(s!=NULL){
        if(strcmp(s->name, name)  == 0){
            s->is_init = init;
            return;
        }
        s=s->next;
    }

    Symbol* new_sym = (Symbol*)malloc(sizeof(Symbol));
    strcpy(new_sym->name, name);
    new_sym->is_init = init;
    new_sym->next = head ; 
    head = new_sym;
}

Symbol* lookup_symbol(char* name){
    Symbol* s = head;
    while (s != NULL){
        if(strcmp(s->name , name) == 0)
        return s;
        s=s->next;
    }
    return NULL;
}

void check_semantics(Node* node){
    if(node == NULL)return;

    if( node->type == NODE_DECL_ASSN){
        check_semantics(node->right);
        add_symbol(node->left->left->val ,1 );
    }

    else if(node ->type == NODE_DECL){
        add_symbol(node->left->val,0);
    }

    else if(node->type == NODE_ID){
        if(lookup_symbol(node->val) == NULL){
            printf("SEMANTIC ERROR: Variable '%s' used before assignment!\n", node->val);
            exit(1);
        }
    }

    else if(node->type == NODE_ASSIGN){
        check_semantics(node->right);
        add_symbol(node->left->val,1);
    }
    else{
        check_semantics(node->left);
        check_semantics(node->right);
    }
    
    if(node->next)
    check_semantics(node->next);

}

void print_symbol_table() {
    Symbol* current = head;
    printf("\n--- SYMBOL TABLE ---\n");
    printf("%-15s | %-10s\n", "Variable", "Initialized");
    printf("-------------------------------\n");
    
    if (current == NULL) {
        printf("(Table is empty)\n");
        return;
    }

    while (current != NULL) {
        printf("%-15s | %-10s\n", 
               current->name, 
               current->is_init ? "Yes" : "No");
        current = current->next;
    }
    printf("-------------------------------\n\n");
}