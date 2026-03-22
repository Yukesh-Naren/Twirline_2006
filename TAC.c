#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tac.h"
#include "ast.h"
#include "semantics.h"

TAC* tacHead = NULL;
TAC* tacTail = NULL;

int tempcount = 1;
int labelcount = 1;

int get_expr_type(Node* root);

TAC* CreateTAC(char* result, char* arg1, char* op, char* arg2, int type)
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

    node->type = type;
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
    appendTAC(CreateTAC("if", cond, "goto", label, TYPE_INT));
}

void emit_goto(char* label)
{
    appendTAC(CreateTAC("goto", label, "", "", TYPE_INT));
}

void emit_label(char* label)
{
    appendTAC(CreateTAC(label, "", "label", "", TYPE_INT));
}

void generate_print(Node* node)
{
    if(node == NULL) return;

    Node* arg = node->left;

    while(arg != NULL)
    {
        if(arg->type == NODE_STRING)
        {
            appendTAC(CreateTAC("print", arg->val, "", "", TYPE_INT));
        }
        else
        {
            char* value = generate_expr(arg);
            appendTAC(CreateTAC("print", value, "", "", get_expr_type(arg)));
        }
        arg = arg->next;
    }
}

int get_expr_type(Node* root)
{
    if (root == NULL) return TYPE_INT;

    if (root->type == NODE_CONST) {
        if(root->val[0] == '\'' && root->val[2] == '\'')
            return TYPE_CHAR;
        if (strchr(root->val, '.') != NULL)
            return TYPE_FLOAT;
        return TYPE_INT;
    }

    if (root->type == NODE_ID) {
        return get_symbol_type(root->val);
    }

    if (root->type == NODE_ARRAY_ACCESS) {
        return get_symbol_type(root->left->val);
    }

    if (root->type == NODE_ASSIGN) {
        return get_symbol_type(root->left->val);
    }

    if (root->type == NODE_ARRAY_ASSIGN) {
        return get_symbol_type(root->left->left->val);
    }

    if (root->type == NODE_OP) {
        if (!strcmp(root->val, "<")  || !strcmp(root->val, ">")  ||
            !strcmp(root->val, "<=") || !strcmp(root->val, ">=") ||
            !strcmp(root->val, "==") || !strcmp(root->val, "!=") ||
            !strcmp(root->val, "&&") || !strcmp(root->val, "||") ||
            !strcmp(root->val, "!")) {
            return TYPE_INT;
        }

        int leftType = get_expr_type(root->left);
        int rightType = get_expr_type(root->right);

        if (leftType == TYPE_FLOAT || rightType == TYPE_FLOAT)
            return TYPE_FLOAT;
        return TYPE_INT;
    }

    return TYPE_INT;
}

char* generate_expr(Node* root)
{
    if(root == NULL)
        return NULL;

    if(root->type == NODE_CONST || root->type == NODE_ID)
    {
        return strdup(root->val);
    }

    if(root->type == NODE_ARRAY_ACCESS)
    {
        char* index = generate_expr(root->right);
        char* temp = newTemp();
        appendTAC(CreateTAC(temp, root->left->val, "=[]", index, get_expr_type(root)));
        return temp;
    }

    if(root->type == NODE_OP)
    {
        if(strcmp(root->val, "!") == 0)
        {
            char* operand = generate_expr(root->left);
            char* temp = newTemp();
            appendTAC(CreateTAC(temp, operand, "!", "", TYPE_INT));
            return temp;
        }

        char* left = generate_expr(root->left);
        char* right = generate_expr(root->right);

        char* temp = newTemp();
        int exprType = get_expr_type(root);
        appendTAC(CreateTAC(temp, left, root->val, right, exprType));

        return temp;
    }

    if(root->type == NODE_ASSIGN)
    {
        char* rhs = generate_expr(root->right);
        appendTAC(CreateTAC(root->left->val, rhs, "=", "", get_symbol_type(root->left->val)));
        return strdup(root->left->val);
    }

    return NULL;
}

void generate_assign(Node* node)
{
    if(node == NULL) return;

    char* rhs = generate_expr(node->right);
    appendTAC(CreateTAC(node->left->val, rhs, "=", "", get_symbol_type(node->left->val)));
}

void generate_array_assign(Node* node)
{
    if(node == NULL) return;

    Node* access = node->left;
    char* index = generate_expr(access->right);
    char* value = generate_expr(node->right);

    appendTAC(CreateTAC(access->left->val, index, "[]=", value, get_symbol_type(access->left->val)));
}

void generate_while(Node* node)
{
    if(node == NULL) return;

    char* startLabel = new_label();
    char* trueLabel  = new_label();
    char* endLabel   = new_label();

    emit_label(startLabel);

    char* cond = generate_expr(node->left);

    emit_if_goto(cond, trueLabel);
    emit_goto(endLabel);

    emit_label(trueLabel);

    if(node->right != NULL)
        generate_stmt_list(node->right);

    emit_goto(startLabel);
    emit_label(endLabel);
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

void generate_input(Node* node)
{
    if(node == NULL || node->left == NULL) return;
    appendTAC(CreateTAC("input", node->left->val, "", "", get_symbol_type(node->left->val)));
}

void generate_stmt(Node* node)
{
    if(node == NULL) return;

    switch(node->type)
    {
        case NODE_DECL:
        case NODE_ARRAY_DECL:
            break;

        case NODE_DECL_ASSN:
            if(node->left != NULL)
                generate_assign(node->left);
            break;

        case NODE_ASSIGN:
            generate_assign(node);
            break;

        case NODE_ARRAY_ASSIGN:
            generate_array_assign(node);
            break;

        case NODE_IF:
            generate_if(node);
            break;

        case NODE_WHILE:
            generate_while(node);
            break;

        case NODE_PRINT:
            generate_print(node);
            break;

        case NODE_INPUT:
            generate_input(node);
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
        else if(strcmp(temp->result, "print") == 0)
            printf("print %s\n", temp->arg1);
        else if(strcmp(temp->result, "input") == 0)
            printf("input %s\n", temp->arg1);
        else if(strcmp(temp->op, "!") == 0)
            printf("%s = ! %s\n", temp->result, temp->arg1);
        else if(strcmp(temp->op, "=") == 0)
            printf("%s = %s\n", temp->result, temp->arg1);
        else if(strcmp(temp->op, "=[]") == 0)
            printf("%s = %s[%s]\n", temp->result, temp->arg1, temp->arg2);
        else if(strcmp(temp->op, "[]=") == 0)
            printf("%s[%s] = %s\n", temp->result, temp->arg1, temp->arg2);
        else
            printf("%s = %s %s %s\n", temp->result, temp->arg1, temp->op, temp->arg2);

        temp = temp->next;
    }

    printf("===============================\n");
}