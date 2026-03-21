// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "include/AST.h"
// #include "include/semantics.h"

// // 1. Your Error Function
// void semanticError(const char *msg) {
//     printf("Semantic Error: %s\n", msg);
//     exit(1);
// }

// // 2. Your Type Checking Function
// char* checkOpType(char* left, char* right, char* op) {
//     if (left == NULL || right == NULL) return "void";

//     // Arithmetic
//     if (!strcmp(op, "+") || !strcmp(op, "-") || !strcmp(op, "*") || !strcmp(op, "/")) {
//         if (!strcmp(left, "int") && !strcmp(right, "int"))
//             return "int";

//         if ((!strcmp(left, "int") && !strcmp(right, "float")) ||
//             (!strcmp(left, "float") && !strcmp(right, "int")) ||
//             (!strcmp(left, "float") && !strcmp(right, "float")))
//             return "float";

//         semanticError("Invalid operands for arithmetic operation");
//     }

//     // Assignment
//     if (!strcmp(op, "=")) {
//         if (strcmp(left, right) != 0)
//             semanticError("Type mismatch in assignment");
//         return left;
//     }

//     semanticError("Unknown operator");
//     return NULL;
// }

// // 3. Determine type of a leaf node (ID or Constant)
// char* getLeafType(Node* node) {
//     if (node->type == NODE_CONST) {
//         return (strchr(node->val, '.') != NULL) ? "float" : "int";
//     }

//     if (node->type == NODE_ID) {
//         Symbol* sym = lookup_symbol(node->val);
//         if (sym == NULL) {
//             printf("Semantic Error: Variable '%s' used before declaration!\n", node->val);
//             exit(1);
//         }
//         return "int"; // Logic: Pull actual type from Symbol Table later
//     }
//     return "void";
// }

// // 4. Recursive Evaluator for math trees (Left/Right)
// char* evaluate_expression(Node* root) {
//     if (root == NULL) return "void";

//     if (root->left == NULL && root->right == NULL) {
//         char* t = getLeafType(root);
//         printf("Leaf: %s -> %s\n", root->val, t);
//         return t;
//     }

//     char* leftT = evaluate_expression(root->left);
//     char* rightT = evaluate_expression(root->right);

//     if (root->type == NODE_OP || root->type == NODE_ASSIGN) {
//         printf("Checking: %s (%s, %s)\n", root->val, leftT, rightT);
//         return checkOpType(leftT, rightT, root->val);
//     }
//     return "void";
// }

// // 5. THE MAIN ENTRY POINT: Walks through lines (Next)
// void check_semantics(Node* root) {
//     Node* current = root;
//     while (current != NULL) {
//         if (current->type == NODE_DECL) {
//             add_symbol(current->left->val, 0); 
//         } 
//         else if (current->type == NODE_DECL_ASSN) {
//             evaluate_expression(current->left); 
//             add_symbol(current->left->left->val, 1);
//         } 
//         else {
//             evaluate_expression(current);
//         }
//         current = current->next; // Move to the next statement
//     }
// }
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/AST.h"
#include "include/semantics.h"

// --- SYMBOL TABLE LOGIC ---
Symbol* symbol_table = NULL;

void add_symbol(char* name, int init) {
    Symbol* s = lookup_symbol(name);
    if (s == NULL) {
        // Create new symbol if it doesn't exist
        s = (Symbol*)malloc(sizeof(Symbol));
        strcpy(s->name, name);
        s->is_init = init;
        s->next = symbol_table;
        symbol_table = s;
    } else {
        // If it exists, just mark it as initialized if needed
        if (init) s->is_init = 1;
    }
}

Symbol* lookup_symbol(char* name) {
    Symbol* s = symbol_table;
    while (s != NULL) {
        if (strcmp(s->name, name) == 0) return s;
        s = s->next;
    }
    return NULL;
}

// --- SEMANTIC ANALYSIS LOGIC ---

void semanticError(const char *msg) {
    printf("Semantic Error: %s\n", msg);
    exit(1);
}

char* checkOpType(char* left, char* right, char* op) {
    if (left == NULL || right == NULL) return "void";

    if (!strcmp(op, "+") || !strcmp(op, "-") || !strcmp(op, "*") || !strcmp(op, "/")) {
        if (!strcmp(left, "int") && !strcmp(right, "int")) return "int";
        if ((!strcmp(left, "int") && !strcmp(right, "float")) ||
            (!strcmp(left, "float") && !strcmp(right, "int")) ||
            (!strcmp(left, "float") && !strcmp(right, "float"))) return "float";
        semanticError("Invalid operands for arithmetic operation");
    }

    if (!strcmp(op, "=")) {
        if (strcmp(left, right) != 0) semanticError("Type mismatch in assignment");
        return left;
    }
    return "void";
}

char* getLeafType(Node* node) {
    if (node->type == NODE_CONST) {
        return (strchr(node->val, '.') != NULL) ? "float" : "int";
    }
    if (node->type == NODE_ID) {
        Symbol* sym = lookup_symbol(node->val);
        if (sym == NULL) {
            printf("Semantic Error: Variable '%s' used before declaration!\n", node->val);
            exit(1);
        }
        return "int"; 
    }
    return "void";
}

char* evaluate_expression(Node* root) {
    if (root == NULL) return "void";
    if (root->left == NULL && root->right == NULL) return getLeafType(root);

    char* leftT = evaluate_expression(root->left);
    char* rightT = evaluate_expression(root->right);

    if (root->type == NODE_OP || root->type == NODE_ASSIGN) {
        return checkOpType(leftT, rightT, root->val);
    }
    return "void";
}


void check_semantics(Node* root) {
    Node* current = root;
    while (current != NULL) {
        if (current->type == NODE_DECL) {
            // Case: x(int);
            // node->left is ID, node->left->left is TYPE
            add_symbol(current->left->val, 0); 
        } 
        else if (current->type == NODE_DECL_ASSN) {
            // Case: t(int) = 5;
            // 1. First, register the variable so it exists in the table
            // In your AST: current->left is '=', current->left->left is ID
            add_symbol(current->left->left->val, 1);
            
            // 2. Now check the right side of the assignment (the expression)
            evaluate_expression(current->left->right);
        } 
        else {
            // Case: standard assignment like a = t + o;
            evaluate_expression(current);
        }
        current = current->next;
    }
}