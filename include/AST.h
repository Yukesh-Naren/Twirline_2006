#include<stdio.h>
#include<stdlib.h>

#ifndef AST_H
#define AST_H

typedef enum{
    NODE_ID,
    NODE_INT_LITERAL,
    NODE_OP,
    NODE_ASSIGN,
    NODE_DECL,
    NODE_DECL_ASSN
    NODE_TYPE,
}NodeType;

typedef struct Node{
    char* val;
    NodeType type;
    struct Node* left;
    struct Node* right;
    struct Node* next;
}Node;

Node* CreateNode(char* val, NodeType type);
void printProgram(Node* root);
void printAST(Node* root , int level);
#endif