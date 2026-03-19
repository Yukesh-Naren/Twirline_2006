#include<stdio.h>
#include<stdlib.h>

#ifndef AST_H
#define AST_H


typedef struct Node{
    char* val;
    struct Node* left;
    struct Node* right;
    struct Node* next;
}Node;

Node* CreateNode(char* val);
void printProgram(Node* root);
void printAST(Node* root , int level);
#endif