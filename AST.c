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

Node* make_array_decl_node(char* name, char* type, char* size) {
    Node* n = CreateNode("ArrayDeclaration", NODE_ARRAY_DECL);

    n->left = CreateNode(name, NODE_ID);
    n->left->left = CreateNode(type, NODE_TYPE);
    n->right = CreateNode(size, NODE_CONST);

    return n;
}

Node* make_array_decl_nd_node(char* name, char* type, Node* dims) {
    Node* n = CreateNode("ArrayDeclaration", NODE_ARRAY_DECL);

    n->left = CreateNode(name, NODE_ID);
    n->left->left = CreateNode(type, NODE_TYPE);
    n->right = dims;

    return n;
}

Node* make_array_access_node(char* name, Node* index) {
    Node* n = CreateNode("ArrayAccess", NODE_ARRAY_ACCESS);

    n->left = CreateNode(name, NODE_ID);
    n->right = index;

    return n;
}

Node* make_array_access_chain_node(Node* base, Node* index) {
    Node* n = CreateNode("ArrayAccess", NODE_ARRAY_ACCESS);

    n->left = base;
    n->right = index;

    return n;
}

Node* make_array_assign_node(Node* access, Node* expr) {
    Node* n = CreateNode("ArrayAssignment", NODE_ARRAY_ASSIGN);

    n->left = access;
    n->right = expr;

    return n;
}

void printAST(Node* root , int level){
    if (root == NULL) return;

    for(int i=0;i<level ;i++)
    printf("  ");

    printf("|-- %s(type=%d)\n",root->val,root->type);
    if(root->left)
    printAST(root ->left, level+1);
    if(root->right)
    printAST(root->right , level+1);
    if(root->next)
    printAST(root->next , level);
}
void printProgram(Node* root){
        printAST(root , 0);
        printf("\n");
    
}
    void freeAST(Node* root){
        if(root == NULL) return ;

        freeAST(root->left);
        freeAST(root->right);
        freeAST(root->next);

        free(root->val);
        free(root);
    }