#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/AST.h"
#include "include/semantics.h"

Symbol* head = NULL;
FunctionSymbol* functionHead = NULL;

static int current_scope_depth = 0;
static int inside_function = 0;
static int current_function_return_type = TYPE_INT;

void semanticError(const char *msg)
{
    printf("Semantic Error: %s\n", msg);
    exit(1);
}

void enter_scope()
{
    current_scope_depth++;
}

void exit_scope()
{
    // Symbol* temp = head;
    // Symbol* prev = NULL;

    // while (temp != NULL)
    // {
    //     if (temp->scope_depth == current_scope_depth)
    //     {
    //         Symbol* toDelete = temp;

    //         if (prev == NULL)
    //             head = temp->next;
    //         else
    //             prev->next = temp->next;

    //         temp = temp->next;

    //         if (toDelete->dim_sizes) free(toDelete->dim_sizes);
    //         if (toDelete->array_values) free(toDelete->array_values);
    //         if (toDelete->array_init) free(toDelete->array_init);
    //         free(toDelete);
    //     }
    //     else
    //     {
    //         prev = temp;
    //         temp = temp->next;
    //     }
    // }

    current_scope_depth--;
}

Symbol* lookup_symbol_current_scope(char* name)
{
    Symbol* temp = head;

    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0 && temp->scope_depth == current_scope_depth)
            return temp;
        temp = temp->next;
    }

    return NULL;
}

Symbol* lookup_symbol(char* name)
{
    Symbol* temp = head;
    Symbol* best = NULL;
    int best_scope = -1;

    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0 && temp->scope_depth >= best_scope)
        {
            best = temp;
            best_scope = temp->scope_depth;
        }
        temp = temp->next;
    }

    return best;
}

int get_symbol_type(const char *name)
{
    if (name == NULL) return TYPE_INT;

    if (strncmp(name, "tmp", 3) == 0)
        return TYPE_INT;

    Symbol* sym = lookup_symbol((char*)name);

    if (sym == NULL) {
        char msg[100];
        sprintf(msg, "Variable '%s' not declared (type lookup)", name);
        semanticError(msg);
    }

    return sym->type;
}

void add_symbol_scoped(char* name, int init, int type, int scope_depth)
{
    if (lookup_symbol_current_scope(name) != NULL)
    {
        char msg[100];
        sprintf(msg, "Variable '%s' already declared in current scope", name);
        semanticError(msg);
    }

    Symbol* newSym = (Symbol*)malloc(sizeof(Symbol));

    strcpy(newSym->name, name);
    newSym->is_init = init;
    newSym->type = type;
    newSym->value = 0.0;

    newSym->is_array = 0;
    newSym->dimensions = 0;
    newSym->dim_sizes = NULL;
    newSym->array_values = NULL;
    newSym->array_init = NULL;
    newSym->scope_depth = scope_depth;

    newSym->next = head;
    head = newSym;
}

void add_symbol(char* name, int init, int type)
{
    add_symbol_scoped(name, init, type, current_scope_depth);
}

void add_array_symbol_nd_scoped(char* name, int type, Node* dims, int scope_depth)
{
    if (lookup_symbol_current_scope(name) != NULL)
    {
        char msg[100];
        sprintf(msg, "Variable '%s' already declared in current scope", name);
        semanticError(msg);
    }

    int count = 0;
    Node* temp = dims;

    while (temp != NULL) {
        count++;
        temp = temp->next;
    }

    if (count == 0)
        semanticError("Array must have at least one dimension");

    Symbol* newSym = (Symbol*)malloc(sizeof(Symbol));
    if (!newSym) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    strcpy(newSym->name, name);
    newSym->is_init = 1;
    newSym->type = type;
    newSym->value = 0.0;

    newSym->is_array = 1;
    newSym->dimensions = count;
    newSym->dim_sizes = (int*)malloc(sizeof(int) * count);
    newSym->scope_depth = scope_depth;

    temp = dims;
    int i = 0;
    int total = 1;

    while (temp != NULL)
    {
        int size = atoi(temp->val);

        if (size <= 0)
            semanticError("Invalid array size");

        newSym->dim_sizes[i++] = size;
        total *= size;
        temp = temp->next;
    }

    newSym->array_values = (float*)calloc(total, sizeof(float));
    newSym->array_init = (int*)calloc(total, sizeof(int));

    newSym->next = head;
    head = newSym;
}

void add_array_symbol_nd(char* name, int type, Node* dims)
{
    add_array_symbol_nd_scoped(name, type, dims, current_scope_depth);
}

/* NEW: add array parameter symbol with unknown sizes */
void add_array_param_symbol_scoped(char* name, int type, Node* dims, int scope_depth)
{
    if (lookup_symbol_current_scope(name) != NULL)
    {
        char msg[100];
        sprintf(msg, "Variable '%s' already declared in current scope", name);
        semanticError(msg);
    }

    int count = 0;
    Node* temp = dims;

    while (temp != NULL) {
        count++;
        temp = temp->next;
    }

    if (count == 0)
        semanticError("Array parameter must have at least one dimension");

    Symbol* newSym = (Symbol*)malloc(sizeof(Symbol));
    if (!newSym) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    strcpy(newSym->name, name);
    newSym->is_init = 1;
    newSym->type = type;
    newSym->value = 0.0;

    newSym->is_array = 1;
    newSym->dimensions = count;
    newSym->dim_sizes = (int*)malloc(sizeof(int) * count);
    newSym->scope_depth = scope_depth;

    for (int i = 0; i < count; i++)
        newSym->dim_sizes[i] = -1;   /* unknown sizes for parameter arrays */

    newSym->array_values = NULL;     /* runtime only */
    newSym->array_init = NULL;

    newSym->next = head;
    head = newSym;
}

void add_array_param_symbol(char* name, int type, Node* dims)
{
    add_array_param_symbol_scoped(name, type, dims, current_scope_depth);
}

const char* type_to_string(int type){
    switch(type){
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_CHAR:return "char";
        default: return "unknown";
    }
}
void print_symbol_table()
{
    printf("\n===== SYMBOL TABLE =====\n");
    printf("%-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
           "NAME", "TYPE","INIT", "VALUE", "ARRAY", "DIMS", "SCOPE", "SIZES");
    printf("-----------------------------------------------------------------------------------------\n");

    Symbol* temp = head;

    while (temp != NULL)
    {
        printf("%-10s %-10s %-10d %-10.2f %-10d %-10d %-10d ",
               temp->name,
               type_to_string(temp->type),
               temp->is_init,
               temp->value,
               temp->is_array,
               temp->dimensions,
               temp->scope_depth);

        if (temp->is_array) {
            for (int i = 0; i < temp->dimensions; i++) {
                if (temp->dim_sizes[i] == -1)
                    printf("?");
                else
                    printf("%d", temp->dim_sizes[i]);

                if (i < temp->dimensions - 1) printf("x");
            }
        }

        printf("\n");
        temp = temp->next;
    }

    printf("=========================================================================================\n");
}

FunctionSymbol* lookup_function(char* name)
{
    FunctionSymbol* temp = functionHead;

    while (temp != NULL)
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }

    return NULL;
}

int count_params(Node* params)
{
    int count = 0;
    Node* temp = params;

    while (temp != NULL)
    {
        count++;
        temp = temp->next;
    }

    return count;
}

int count_args(Node* args)
{
    int count = 0;
    Node* temp = args;

    while (temp != NULL)
    {
        count++;
        temp = temp->next;
    }

    return count;
}

void add_function(char* name, Node* params, int return_type)
{
    if (lookup_function(name) != NULL)
    {
        char msg[100];
        sprintf(msg, "Function '%s' already declared", name);
        semanticError(msg);
    }

    FunctionSymbol* fn = (FunctionSymbol*)malloc(sizeof(FunctionSymbol));
    strcpy(fn->name, name);
    fn->param_count = count_params(params);
    fn->return_type = return_type;
    fn->params = params;
    fn->next = functionHead;
    functionHead = fn;
}

int get_node_type_from_decl(Node* typeNode)
{
    if (strcmp(typeNode->val, "int") == 0) return TYPE_INT;
    if (strcmp(typeNode->val, "float") == 0) return TYPE_FLOAT;
    if (strcmp(typeNode->val, "char") == 0) return TYPE_CHAR;

    semanticError("Unknown declaration type");
    return TYPE_INT;
}

void validate_assignment_type(Symbol* sym, float value)
{
    if (sym->type == TYPE_INT && value != (int)value)
        semanticError("Cannot assign float to int variable");

    if (sym->type == TYPE_CHAR) {
        if (value != (int)value || value < 0 || value > 255)
            semanticError("Invalid char assignment");
    }
}

float evaluate_expression(Node* node);

int compute_offset(Symbol* sym, Node* access)
{
    int indices[20];
    int k = 0;
    Node* temp = access;

    while (temp != NULL && temp->type == NODE_ARRAY_ACCESS)
    {
        indices[k++] = (int)evaluate_expression(temp->right);
        temp = temp->left;
    }

    if (temp == NULL || temp->type != NODE_ID)
        semanticError("Invalid array access structure");

    if (strcmp(temp->val, sym->name) != 0)
        semanticError("Array name mismatch in access");

    if (k != sym->dimensions)
        semanticError("Array dimension mismatch");

    int offset = 0;
    int multiplier = 1;

    for (int i = k - 1; i >= 0; i--)
    {
        int idx = indices[i];

        if (sym->dim_sizes[i] != -1) {
            if (idx < 0 || idx >= sym->dim_sizes[i])
                semanticError("Array index out of bounds");
        } else {
            if (idx < 0)
                semanticError("Array index cannot be negative");
        }

        offset += idx * multiplier;

        if (sym->dim_sizes[i] != -1)
            multiplier *= sym->dim_sizes[i];
        else
            multiplier *= 1;
    }

    return offset;
}

float evaluate_expression(Node* node)
{
    if (node == NULL) return 0;

    if (node->type == NODE_CONST)
        return atof(node->val);

    if (node->type == NODE_ID)
    {
        Symbol* sym = lookup_symbol(node->val);

        if (sym == NULL)
        {
            char msg[100];
            sprintf(msg, "Undeclared variable '%s'", node->val);
            semanticError(msg);
        }

        return sym->value;
    }

    if (node->type == NODE_ARRAY_ACCESS)
    {
        Node* temp = node;

        while (temp->type == NODE_ARRAY_ACCESS)
            temp = temp->left;

        if (temp == NULL || temp->type != NODE_ID)
            semanticError("Invalid array access");

        Symbol* sym = lookup_symbol(temp->val);

        if (!sym)
            semanticError("Undeclared array");

        if (!sym->is_array)
            semanticError("Variable is not an array");

        int offset = compute_offset(sym, node);

        if (sym->array_values == NULL)
        {
            /* array parameter: runtime access only */
            return 0;
        }

        return sym->array_values[offset];
    }

    if (node->type == NODE_FUNC_CALL)
    {
        FunctionSymbol* fn = lookup_function(node->left->val);
        Node* arg;
        Node* param;

        if (fn == NULL) {
            char msg[100];
            sprintf(msg, "Function '%s' not declared", node->left->val);
            semanticError(msg);
        }

        int argc = count_args(node->right);
        if (argc != fn->param_count) {
            char msg[100];
            sprintf(msg, "Function '%s' expects %d argument(s), got %d",
                    node->left->val, fn->param_count, argc);
            semanticError(msg);
        }

        arg = node->right;
        param = fn->params;

        while (arg != NULL && param != NULL) {
            Node* param_dims = NULL;

            if (param->type != NODE_PARAM || param->left == NULL)
                semanticError("Malformed function parameter");

            param_dims = param->left->right;

            if (param_dims != NULL) {
                if (arg->left->type != NODE_ID)
                    semanticError("Array parameter requires array argument");

                Symbol* argSym = lookup_symbol(arg->left->val);

                if (!argSym)
                    semanticError("Undeclared array argument");

                if (!argSym->is_array)
                    semanticError("Argument is not an array");

                int param_dim_count = 0;
                Node* d = param_dims;
                while (d != NULL) {
                    param_dim_count++;
                    d = d->next;
                }

                if (argSym->dimensions != param_dim_count)
                    semanticError("Array argument dimension mismatch");
            } else {
                evaluate_expression(arg->left);
            }

            arg = arg->next;
            param = param->next;
        }

        return 0;
    }

    if (node->type == NODE_OP)
    {
        if (!strcmp(node->val, "!")) {
            float val = evaluate_expression(node->left);
            return !val;
        }

        float left = evaluate_expression(node->left);
        float right = evaluate_expression(node->right);

        if (!strcmp(node->val, "+")) return left + right;
        if (!strcmp(node->val, "-")) return left - right;
        if (!strcmp(node->val, "*")) return left * right;
        if (!strcmp(node->val, "/")) {
            if (right == 0) semanticError("Division by zero");
            return left / right;
        }

        if (!strcmp(node->val, "<"))  return left < right;
        if (!strcmp(node->val, ">"))  return left > right;
        if (!strcmp(node->val, "<=")) return left <= right;
        if (!strcmp(node->val, ">=")) return left >= right;
        if (!strcmp(node->val, "==")) return left == right;
        if (!strcmp(node->val, "!=")) return left != right;
        if (!strcmp(node->val, "&&")) return left && right;
        if (!strcmp(node->val, "||")) return left || right;
    }

    return 0;
}

static void declare_function_params(Node* params)
{
    Node* temp = params;

    while (temp != NULL)
    {
        if (temp->type != NODE_PARAM)
            semanticError("Invalid parameter node");

        if (temp->left == NULL || temp->left->left == NULL)
            semanticError("Malformed parameter declaration");

        char* name = temp->left->val;
        int type = get_node_type_from_decl(temp->left->left);
        Node* dims = temp->left->right;

        if (dims == NULL)
        {
            add_symbol(name, 1, type);
        }
        else
        {
            add_array_param_symbol(name, type, dims);
        }

        temp = temp->next;
    }
}

static void check_semantics_block(Node* root);

static void check_semantics_function(Node* fn)
{
    if (fn == NULL || fn->type != NODE_FUNC_DEF)
        semanticError("Invalid function definition");

    if (fn->left == NULL)
        semanticError("Malformed function definition");

    char* name = fn->left->val;
    Node* params = fn->left->left;
    Node* body = fn->right;

    add_function(name, params, TYPE_INT);

    inside_function++;
    current_function_return_type = TYPE_INT;

    enter_scope();
    declare_function_params(params);
    check_semantics_block(body);
    exit_scope();

    inside_function--;
}

static void check_semantics_block(Node* root)
{
    Node* current = root;

    while (current != NULL)
    {
        if (current->type == NODE_DECL)
        {
            char* name = current->left->val;
            int type = get_node_type_from_decl(current->left->left);
            add_symbol(name, 0, type);
        }
        else if (current->type == NODE_ARRAY_DECL)
        {
            char* name = current->left->val;
            int type = get_node_type_from_decl(current->left->left);
            add_array_symbol_nd(name, type, current->right);
        }
        else if (current->type == NODE_ASSIGN)
        {
            Symbol* sym = lookup_symbol(current->left->val);

            if (!sym)
                semanticError("Undeclared variable");

            if (sym->is_array)
                semanticError("Cannot assign directly to array name");

            float val = evaluate_expression(current->right);
            validate_assignment_type(sym, val);

            sym->value = val;
            sym->is_init = 1;
        }
        else if (current->type == NODE_ARRAY_ASSIGN)
        {
            Node* access = current->left;
            Node* temp = access;

            while (temp->type == NODE_ARRAY_ACCESS)
                temp = temp->left;

            if (temp == NULL || temp->type != NODE_ID)
                semanticError("Invalid array assignment target");

            Symbol* sym = lookup_symbol(temp->val);

            if (!sym)
                semanticError("Undeclared array");

            if (!sym->is_array)
                semanticError("Variable is not an array");

            int offset = compute_offset(sym, access);

            float val = evaluate_expression(current->right);
            validate_assignment_type(sym, val);

            if (sym->array_values != NULL && sym->array_init != NULL) {
                sym->array_values[offset] = val;
                sym->array_init[offset] = 1;
            }
        }
        else if (current->type == NODE_RETURN)
        {
            if (!inside_function)
                semanticError("Return statement outside function");

            if (current->left != NULL)
                evaluate_expression(current->left);
        }
        else if (current->type == NODE_FUNC_CALL)
        {
            evaluate_expression(current);
        }
        else if (current->type == NODE_MAIN)
        {
            semanticError("Nested main block not allowed");
        }
        else if (current->type == NODE_FUNC_DEF)
        {
            semanticError("Nested function definition not allowed");
        }
        else
        {
            evaluate_expression(current);
        }

        current = current->next;
    }
}

void check_semantics(Node* root)
{
    Node* current = root;
    int main_count = 0;

    while (current != NULL)
    {
        if (current->type == NODE_FUNC_DEF)
        {
            check_semantics_function(current);
        }
        else if (current->type == NODE_MAIN)
        {
            main_count++;

            if (main_count > 1)
                semanticError("Multiple main blocks not allowed");

            enter_scope();
            check_semantics_block(current->left);
            exit_scope();
        }
        else
        {
            semanticError("Only function definitions and main block allowed at top level");
        }

        current = current->next;
    }

    if (main_count == 0)
        semanticError("No main block found");
}