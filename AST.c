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
    node->next =NULL;
    return node;
}

void printAST(Node* root , int level){
    if (root == NULL) return;

    for(int i=0;i<level ;i++)
    printf("  ");

    printf("|-- %s(type=%d)\n",root->val,root->type);
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
    void freeAST(Node* root){
        if(root == NULL) return ;

        freeAST(root->left);
        freeAST(root->right);
        freeAST(root->next);

        free(root->val);
        free(root);
    }