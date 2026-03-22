#include<stdio.h>
#include<stdlib.h>

#ifndef AST_H
#define AST_H

typedef enum{
    NODE_ID,
    NODE_CONST,
    NODE_OP,
    NODE_ASSIGN,
    NODE_DECL,
    NODE_DECL_ASSN,
    NODE_TYPE,
    NODE_IF,
    NODE_ELSE,
    NODE_WHILE,
    NODE_STRING,
    NODE_PRINT,
    NODE_INPUT,
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
void freeAST(Node* root);
#endif