#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantics.h"

/*
Assumption:
- Identifiers already have a type assigned somewhere (symbol table or manually)
- Constants are detected using NodeType
*/

void semanticError(const char *msg)
{
    printf("Semantic Error: %s\n", msg);
    exit(1);
}

/* Determine type of a leaf node */
char* getLeafType(Node* node)
{
    if (node->type == NODE_CONST)
    {
        // simple check: if contains '.' → float else int
        if (strchr(node->val, '.') != NULL)
            return "float";
        else
            return "int";
    }

    if (node->type == NODE_ID)
    {
        // For now assume identifiers are int (or extend using symbol table)
        return "int";
    }

    return NULL;
}

/* Type checking for operations */
char* checkOpType(char* left, char* right, char* op)
{
    // Arithmetic
    if (!strcmp(op, "+") || !strcmp(op, "-") ||
        !strcmp(op, "*") || !strcmp(op, "/"))
    {
        if (!strcmp(left, "int") && !strcmp(right, "int"))
            return "int";

        if ((!strcmp(left, "int") && !strcmp(right, "float")) ||
            (!strcmp(left, "float") && !strcmp(right, "int")) ||
            (!strcmp(left, "float") && !strcmp(right, "float")))
            return "float";

        semanticError("Invalid operands for arithmetic operation");
    }

    // Assignment
    if (!strcmp(op, "="))
    {
        if (strcmp(left, right) != 0)
            semanticError("Type mismatch in assignment");

        return left;
    }

    semanticError("Unknown operator");
    return NULL;
}

/* Main semantic analyzer */
char* analyze(Node* root)
{
    if (root == NULL)
        return NULL;

    // Handle statement list using next pointer
    if (root->next != NULL)
        analyze(root->next);

    // Leaf node
    if (root->left == NULL && root->right == NULL)
    {
        char* t = getLeafType(root);

        printf("Leaf: %s -> %s\n", root->val, t);

        return t;
    }

    // Analyze children
    char* leftType = analyze(root->left);
    char* rightType = analyze(root->right);

    printf("Checking: %s (%s , %s)\n", root->val, leftType, rightType);

    // Operation node
    if (root->type == NODE_OP || root->type == NODE_ASSIGN)
    {
        char* result = checkOpType(leftType, rightType, root->val);
        return result;
    }

    return NULL;
}