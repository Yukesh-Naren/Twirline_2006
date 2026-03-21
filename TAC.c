#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tac.h"
#include "ast.h"

TAC* tacHead = NULL;
TAC* tacTail = NULL;
int tempcount = 1;

TAC* CreateTAC(char* result,char* arg1, char* op , char* arg2)
{
    TAC* node = (TAC*)malloc(sizeof(TAC));
    strcpy(node->result, result);
    strcpy(node->arg1,arg1);

    if(op!=NULL)
    strcpy(node->op,op);
    else
    node->op[0] = '\0';

    if(arg2!=NULL)
    strcpy(node->arg2,arg2);
    else
    node->arg2[0]='\0';

    node->next = NULL;
    return node;
}

void appendTAC(TAC* node){
    if (tacHead ==NULL)
    tacHead = tacTail =node;
    else{
        tacTail->next  = node;
        tacTail = node;
    }
}

char* newTemp(){
    static char temp[20];
    char* name = (char*)malloc(20);

    sprintf(temp,"t%d",tempcount++);
    strcpy(name, temp);

    return name;
}

char* generateTAC(Node* root){
    if (root == NULL)
    {
        printf("NULL node in TAC\n");
        return NULL;
    }

    printf("Visiting node type=%d, val=%s\n", root->type, root->val);
    // if(root == NULL)
    // return NULL;

    if(root->type == NODE_CONST || root->type == NODE_ID)
    {   
         printf("Leaf node returning: %s\n", root->val); 
        return root->val;
    }
    else if(root->type == NODE_OP)
    {
        char* left = generateTAC(root->left);
        char* right = generateTAC(root->right);

        char* temp = newTemp();
        appendTAC(CreateTAC(temp, left , root->val, right));

        return temp;
    }

    if(root->type == NODE_ASSIGN){
        char* rhs = generateTAC(root->right);
        printf("Emitting: %s = %s\n", root->left->val, rhs);
        appendTAC(CreateTAC(root->left->val, rhs , "=", ""));
        return root->left->val;
    }

    if(root->type == NODE_DECL_ASSN){
        char* rhs = generateTAC(root->left->right);
        printf("Emitting: %s = %s\n", root->left->left->val, rhs);
        appendTAC(CreateTAC(root->left->left->val,rhs,"=",""));
        return root->left->left->val;
    }
    printf("No matching TAC rule for node type =%d\n",root->type);
    return NULL;
}

void print_TAC(){
    TAC* temp = tacHead;

    printf("\n======THREE ADDRESS CODE=======\n");

    while (temp!=NULL)
    {
        if(strcmp(temp->op,"=") == 0)
        printf("%s = %s\n",temp->result, temp->arg1);
        else
        printf("%s = %s %s %s \n",temp->result,temp->arg1,temp->op,temp->arg2);
        temp = temp->next;
    }

    printf("=========================\n");
}