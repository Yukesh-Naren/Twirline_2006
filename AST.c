#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"include/AST.h"

Node* CreateNode(char* val, NodeType type){
    Node* node = (Node*)malloc(sizeof(Node));
    node->val = strdup(val);
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void printAST(Node* root , int level){
    if (root == NULL) return;

    for(int i=0;i<level ;i++)
    printf(" ");

    printf("|-- %s\n",root->val);
    printAST(root ->left, level+1);
    printAST(root->right , level+1);
}
void printProgram(Node* root){
    while (root !=NULL){
        printAST(root , 0);
        printf("\n");
        root = root->next;
    }
}