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
    NODE_ARRAY_DECL,
    NODE_ARRAY_ACCESS,
    NODE_ARRAY_ASSIGN
}NodeType;

typedef struct Node{
    char* val;
    NodeType type;
    struct Node* left;
    struct Node* right;
    struct Node* next;
}Node;

Node* make_array_decl_node(char* name , char* type, char* size);
Node* make_array_access_node(char* name, Node* indexNode);
Node* make_array_assign_node(Node* accessNode, Node* valueNode);
Node* CreateNode(char* val, NodeType type);
void printProgram(Node* root);
void printAST(Node* root , int level);
void freeAST(Node* root);
#endif