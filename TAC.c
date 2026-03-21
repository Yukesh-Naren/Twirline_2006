#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tac.h"
#include "ast.h"

TAC* tacHead = NULL;
TAC* tacTail = NULL;

int tempcount = 1;
int labelcount = 1;

TAC* CreateTAC(char* result, char* arg1, char* op, char* arg2)
{
    TAC* node = (TAC*)malloc(sizeof(TAC));

    strcpy(node->result, result ? result : "");
    strcpy(node->arg1, arg1 ? arg1 : "");

    if(op != NULL)
        strcpy(node->op, op);
    else
        node->op[0] = '\0';

    if(arg2 != NULL)
        strcpy(node->arg2, arg2);
    else
        node->arg2[0] = '\0';

    node->next = NULL;
    return node;
}

void appendTAC(TAC* node)
{
    if(tacHead == NULL)
        tacHead = tacTail = node;
    else {
        tacTail->next = node;
        tacTail = node;
    }
}

char* newTemp()
{
    static char temp[20];
    char* name = (char*)malloc(20);

    sprintf(temp, "tmp%d", tempcount++);
    strcpy(name, temp);

    return name;
}

char* new_label()
{
    static char label[20];
    char* name = (char*)malloc(20);

    sprintf(label, "L%d", labelcount++);
    strcpy(name, label);

    return name;
}

void emit_if_goto(char* cond, char* label)
{
    appendTAC(CreateTAC("if", cond, "goto", label));
}

void emit_goto(char* label)
{
    appendTAC(CreateTAC("goto", label, "", ""));
}

void emit_label(char* label)
{
    appendTAC(CreateTAC(label, "", "label", ""));
}


char* generate_expr(Node* root)
{
    if(root == NULL)
        return NULL;

    if(root->type == NODE_CONST || root->type == NODE_ID)
    {
        return strdup(root->val);
    }

    if(root->type == NODE_OP)
    {
        if(strcmp(root->val, "!") == 0)
        {
            char* operand = generate_expr(root->left);
            char* temp = newTemp();
            appendTAC(CreateTAC(temp, operand, "!", ""));
            return temp;
        }

        char* left = generate_expr(root->left);
        char* right = generate_expr(root->right);

        char* temp = newTemp();
        appendTAC(CreateTAC(temp, left, root->val, right));

        return temp;
    }

    if(root->type == NODE_ASSIGN)
    {
        char* rhs = generate_expr(root->right);
        appendTAC(CreateTAC(root->left->val, rhs, "=", ""));
        return strdup(root->left->val);
    }

    return NULL;
}

void generate_assign(Node* node)
{
    if(node == NULL) return;

    char* rhs = generate_expr(node->right);
    appendTAC(CreateTAC(node->left->val, rhs, "=", ""));
}

void generate_if(Node* node)
{
    if(node == NULL) return;

    char* trueLabel = new_label();
    char* falseLabel = new_label();
    char* endLabel = new_label();

    char* cond = generate_expr(node->left);

    emit_if_goto(cond, trueLabel);
    emit_goto(falseLabel);

    emit_label(trueLabel);

    if(node->right != NULL && node->right->type == NODE_ELSE)
        generate_stmt_list(node->right->left);
    else
        generate_stmt(node->right);

    emit_goto(endLabel);

    emit_label(falseLabel);

    if(node->right != NULL && node->right->type == NODE_ELSE)
    {
        Node* elsePart = node->right->right;

        if(elsePart != NULL)
        {
            if(elsePart->type == NODE_IF)
                generate_if(elsePart);
            else
                generate_stmt_list(elsePart);
        }
    }

    emit_label(endLabel);
}

void generate_stmt(Node* node)
{
    if(node == NULL) return;

    switch(node->type)
    {
        case NODE_DECL:
            break;

        case NODE_DECL_ASSN:
            if(node->left != NULL)
                generate_assign(node->left);
            break;

        case NODE_ASSIGN:
            generate_assign(node);
            break;

        case NODE_IF:
            generate_if(node);
            break;

        default:
            generate_expr(node);
            break;
    }
}

void generate_stmt_list(Node* node)
{
    Node* curr = node;
    while(curr != NULL)
    {
        generate_stmt(curr);
        curr = curr->next;
    }
}

void generate_TAC(Node* root)
{
    tacHead = tacTail = NULL;
    tempcount = 1;
    labelcount = 1;
    generate_stmt_list(root);
}

void print_TAC()
{
    TAC* temp = tacHead;

    printf("\n====== THREE ADDRESS CODE ======\n");

    while(temp != NULL)
    {
        if(strcmp(temp->result, "if") == 0 && strcmp(temp->op, "goto") == 0)
            printf("if %s goto %s\n", temp->arg1, temp->arg2);

        else if(strcmp(temp->result, "goto") == 0)
            printf("goto %s\n", temp->arg1);

        else if(strcmp(temp->op, "label") == 0)
            printf("%s:\n", temp->result);

        else if(strcmp(temp->op, "!") == 0)
            printf("%s = ! %s\n", temp->result, temp->arg1);

        else if(strcmp(temp->op, "=") == 0)
            printf("%s = %s\n", temp->result, temp->arg1);

        else
            printf("%s = %s %s %s\n", temp->result, temp->arg1, temp->op, temp->arg2);

        temp = temp->next;
    }

    printf("===============================\n");
}