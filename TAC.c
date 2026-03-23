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

static void tac_error(const char* msg)
{
    printf("TAC Error: %s\n", msg);
    exit(1);
}

static int count_arg_nodes(Node* args)
{
    int count = 0;
    Node* temp = args;

    while (temp != NULL) {
        count++;
        temp = temp->next;
    }

    return count;
}

static int count_param_nodes(Node* params)
{
    int count = 0;
    Node* temp = params;

    while (temp != NULL) {
        count++;
        temp = temp->next;
    }

    return count;
}

static int is_array_symbol_name(const char* name)
{
    Symbol* sym;

    if (name == NULL)
        return 0;

    sym = lookup_symbol((char*)name);
    if (sym != NULL)
        return sym->is_array;

    return 0;
}

static int is_array_param_name(const char* name)
{
    FunctionSymbol* fn = functionHead;

    while (fn != NULL) {
        Node* p = fn->params;
        while (p != NULL) {
            if (p->type == NODE_PARAM && p->left != NULL) {
                if (strcmp(p->left->val, name) == 0) {
                    if (p->left->right != NULL)
                        return 1;
                    return 0;
                }
            }
            p = p->next;
        }
        fn = fn->next;
    }

    return 0;
}

static int get_param_type_by_name(const char* name)
{
    FunctionSymbol* fn = functionHead;

    while (fn != NULL) {
        Node* p = fn->params;
        while (p != NULL) {
            if (p->left && strcmp(p->left->val, name) == 0) {
                if (p->left->left == NULL)
                    tac_error("malformed function parameter type");
                return get_node_type_from_decl(p->left->left);
            }
            p = p->next;
        }
        fn = fn->next;
    }

    return -1;
}

static int get_identifier_type_by_name(const char* name)
{
    Symbol* sym;

    if (name == NULL)
        return TYPE_INT;

    if (strncmp(name, "tmp", 3) == 0)
        return TYPE_INT;

    sym = lookup_symbol((char*)name);
    if (sym != NULL)
        return sym->type;

    {
        int ptype = get_param_type_by_name(name);
        if (ptype != -1)
            return ptype;
    }

    {
        FunctionSymbol* fn = lookup_function((char*)name);
        if (fn != NULL)
            return fn->return_type;
    }

    {
        char msg[100];
        sprintf(msg, "Variable '%s' not declared (type lookup)", name);
        semanticError(msg);
    }

    return TYPE_INT;
}

TAC* CreateTAC(char* result, char* arg1, char* op, char* arg2, int type)
{
    TAC* node = (TAC*)malloc(sizeof(TAC));

    strcpy(node->result, result ? result : "");
    strcpy(node->arg1, arg1 ? arg1 : "");

    if (op != NULL)
        strcpy(node->op, op);
    else
        node->op[0] = '\0';

    if (arg2 != NULL)
        strcpy(node->arg2, arg2);
    else
        node->arg2[0] = '\0';

    node->type = type;
    node->next = NULL;
    return node;
}

void appendTAC(TAC* node)
{
    if (node == NULL)
        return;

    if (tacHead == NULL)
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

char* get_array_base_name(Node* access)
{
    Node* temp = access;

    while (temp != NULL && temp->type == NODE_ARRAY_ACCESS)
        temp = temp->left;

    if (temp == NULL || temp->type != NODE_ID)
        tac_error("invalid array base");

    return temp->val;
}

char* generate_expr(Node* root);
void generate_stmt(Node* node);
void generate_stmt_list(Node* node);
void generate_main(Node* node);

char* generate_array_offset(Node* access)
{
    char* base = get_array_base_name(access);
    Symbol* sym = lookup_symbol(base);

    Node* indexNodes[20];
    int k = 0;
    Node* temp = access;

    if (sym == NULL)
        tac_error("array not declared");

    while (temp != NULL && temp->type == NODE_ARRAY_ACCESS)
    {
        indexNodes[k++] = temp->right;
        temp = temp->left;
    }

    if (k != sym->dimensions)
        tac_error("array dimension mismatch");

    for (int i = 0; i < k / 2; i++) {
        Node* t = indexNodes[i];
        indexNodes[i] = indexNodes[k - 1 - i];
        indexNodes[k - 1 - i] = t;
    }

    {
        char* offset = strdup("0");

        for (int i = 0; i < k; i++)
        {
            char* idx = generate_expr(indexNodes[i]);
            int mul = 1;

            /* For declared arrays, dim sizes are known.
               For array parameters, unknown sizes may be -1.
               In that case this falls back to 1.
               Proper ND parameter indexing still needs size metadata
               passed separately in semantics/codegen. */
            for (int j = i + 1; j < sym->dimensions; j++) {
                if (sym->dim_sizes[j] > 0)
                    mul *= sym->dim_sizes[j];
                else
                    mul *= 1;
            }

            {
                char mulStr[20];
                char* t1;
                char* t2;

                sprintf(mulStr, "%d", mul);

                t1 = newTemp();
                appendTAC(CreateTAC(t1, idx, "*", mulStr, TYPE_INT));

                t2 = newTemp();
                appendTAC(CreateTAC(t2, offset, "+", t1, TYPE_INT));

                offset = t2;
            }
        }

        return offset;
    }
}

int get_expr_type(Node* root)
{
    if (root == NULL) return TYPE_INT;

    if (root->type == NODE_CONST) {
        if (root->val[0] == '\'' && root->val[2] == '\'')
            return TYPE_CHAR;
        if (strchr(root->val, '.') != NULL)
            return TYPE_FLOAT;
        return TYPE_INT;
    }

    if (root->type == NODE_ID)
        return get_identifier_type_by_name(root->val);

    if (root->type == NODE_ARRAY_ACCESS) {
        char* base = get_array_base_name(root);
        return get_identifier_type_by_name(base);
    }

    if (root->type == NODE_ASSIGN)
        return get_identifier_type_by_name(root->left->val);

    if (root->type == NODE_ARRAY_ASSIGN) {
        char* base = get_array_base_name(root->left);
        return get_identifier_type_by_name(base);
    }

    if (root->type == NODE_FUNC_CALL) {
        FunctionSymbol* fn = lookup_function(root->left->val);
        if (fn == NULL)
            tac_error("function not declared");
        return fn->return_type;
    }

    if (root->type == NODE_RETURN) {
        if (root->left == NULL) return TYPE_INT;
        return get_expr_type(root->left);
    }

    if (root->type == NODE_OP) {
        if (!strcmp(root->val, "<")  || !strcmp(root->val, ">")  ||
            !strcmp(root->val, "<=") || !strcmp(root->val, ">=") ||
            !strcmp(root->val, "==") || !strcmp(root->val, "!=") ||
            !strcmp(root->val, "&&") || !strcmp(root->val, "||") ||
            !strcmp(root->val, "!"))
            return TYPE_INT;

        {
            int leftType = get_expr_type(root->left);
            int rightType = get_expr_type(root->right);

            if (leftType == TYPE_FLOAT || rightType == TYPE_FLOAT)
                return TYPE_FLOAT;
            return TYPE_INT;
        }
    }

    return TYPE_INT;
}

char* generate_expr(Node* root)
{
    if (root == NULL)
        return NULL;

    if (root->type == NODE_CONST)
        return strdup(root->val);

    if (root->type == NODE_ID)
        return strdup(root->val);

    if (root->type == NODE_ARRAY_ACCESS)
    {
        char* base = get_array_base_name(root);
        char* offset = generate_array_offset(root);
        char* temp = newTemp();

        appendTAC(CreateTAC(temp, base, "=[]", offset, get_expr_type(root)));
        return temp;
    }

    if (root->type == NODE_FUNC_CALL)
    {
        Node* arg = root->right;
        int argc = count_arg_nodes(arg);

        while (arg != NULL) {
            if (arg->type != NODE_ARG)
                tac_error("invalid function argument node");

            /* If whole array variable is passed, pass base name directly */
            if (arg->left != NULL &&
                arg->left->type == NODE_ID &&
                is_array_symbol_name(arg->left->val))
            {
                appendTAC(CreateTAC("param", arg->left->val, "", "", get_expr_type(arg->left)));
            }
            else
            {
                char* value = generate_expr(arg->left);
                appendTAC(CreateTAC("param", value, "", "", get_expr_type(arg->left)));
            }

            arg = arg->next;
        }

        {
            char argcStr[20];
            char* temp = newTemp();
            sprintf(argcStr, "%d", argc);
            appendTAC(CreateTAC(temp, root->left->val, "call", argcStr, get_expr_type(root)));
            return temp;
        }
    }

    if (root->type == NODE_OP)
    {
        if (strcmp(root->val, "!") == 0)
        {
            char* operand = generate_expr(root->left);
            char* temp = newTemp();
            appendTAC(CreateTAC(temp, operand, "!", "", TYPE_INT));
            return temp;
        }

        {
            char* left = generate_expr(root->left);
            char* right = generate_expr(root->right);
            char* temp = newTemp();
            int exprType = get_expr_type(root);

            appendTAC(CreateTAC(temp, left, root->val, right, exprType));
            return temp;
        }
    }

    if (root->type == NODE_ASSIGN)
    {
        char* rhs = generate_expr(root->right);
        appendTAC(CreateTAC(root->left->val, rhs, "=", "", get_expr_type(root->left)));
        return strdup(root->left->val);
    }

    tac_error("unsupported expression node");
    return NULL;
}

void generate_assign(Node* node)
{
    if (node == NULL)
        return;

    {
        char* rhs = generate_expr(node->right);
        appendTAC(CreateTAC(node->left->val, rhs, "=", "", get_expr_type(node->left)));
    }
}

void generate_array_assign(Node* node)
{
    if (node == NULL)
        return;

    {
        Node* access = node->left;
        char* base = get_array_base_name(access);
        char* offset = generate_array_offset(access);
        char* value = generate_expr(node->right);

        appendTAC(CreateTAC(base, offset, "[]=", value, get_identifier_type_by_name(base)));
    }
}

void generate_return(Node* node)
{
    if (node == NULL)
        return;

    if (node->left == NULL) {
        appendTAC(CreateTAC("return", "", "", "", TYPE_INT));
        return;
    }

    {
        char* value = generate_expr(node->left);
        appendTAC(CreateTAC("return", value, "", "", get_expr_type(node->left)));
    }
}

static void generate_function_params(Node* params)
{
    Node* p = params;
    int index = 0;

    while (p != NULL)
    {
        char idxStr[20];

        if (p->type != NODE_PARAM || p->left == NULL)
            tac_error("malformed function parameter");

        sprintf(idxStr, "%d", index);

        /* Works for both scalar and array params.
           For array params this means: receive base address/reference. */
        appendTAC(CreateTAC(p->left->val, idxStr, "getparam", "", get_node_type_from_decl(p->left->left)));

        index++;
        p = p->next;
    }
}

void generate_function_def(Node* node)
{
    if (node == NULL)
        return;

    if (node->left == NULL || node->left->type != NODE_ID)
        tac_error("malformed function definition");

    appendTAC(CreateTAC(node->left->val, "", "func", "", TYPE_INT));

    /* NEW: receive parameters at function entry */
    generate_function_params(node->left->left);

    if (node->right != NULL)
        generate_stmt_list(node->right);

    appendTAC(CreateTAC(node->left->val, "", "endfunc", "", TYPE_INT));
}

void generate_main(Node* node)
{
    if (node == NULL)
        return;

    appendTAC(CreateTAC("main", "", "func", "", TYPE_INT));

    if (node->left != NULL)
        generate_stmt_list(node->left);

    appendTAC(CreateTAC("main", "", "endfunc", "", TYPE_INT));
}

void generate_while(Node* node)
{
    if (node == NULL)
        return;

    {
        char* startLabel = new_label();
        char* trueLabel  = new_label();
        char* endLabel   = new_label();

        emit_label(startLabel);

        {
            char* cond = generate_expr(node->left);
            emit_if_goto(cond, trueLabel);
            emit_goto(endLabel);
        }

        emit_label(trueLabel);

        if (node->right != NULL)
            generate_stmt_list(node->right);

        emit_goto(startLabel);
        emit_label(endLabel);
    }
}

void generate_if(Node* node)
{
    if (node == NULL)
        return;

    {
        char* trueLabel = new_label();
        char* falseLabel = new_label();
        char* endLabel = new_label();
        char* cond = generate_expr(node->left);

        emit_if_goto(cond, trueLabel);
        emit_goto(falseLabel);

        emit_label(trueLabel);

        if (node->right != NULL) {
            if (node->right->type == NODE_ELSE)
            generate_stmt_list(node->right->left);
            else
            generate_stmt_list(node->right);   // ✅ FIX HERE
        }

        emit_goto(endLabel);
        emit_label(falseLabel);

        if (node->right != NULL && node->right->type == NODE_ELSE)
        {
            Node* elsePart = node->right->right;

            if (elsePart != NULL)
            {
                if (elsePart->type == NODE_IF)
                    generate_if(elsePart);
                else
                    generate_stmt_list(elsePart);
            }
        }

        emit_label(endLabel);
    }
}

void generate_input(Node* node)
{
    if (node == NULL || node->left == NULL)
        return;

    appendTAC(CreateTAC("input", node->left->val, "", "", get_expr_type(node->left)));
}

void generate_print(Node* node)
{
    if (node == NULL)
        return;

    {
        Node* arg = node->left;

        while (arg != NULL)
        {
            if (arg->type == NODE_STRING)
                appendTAC(CreateTAC("print", arg->val, "", "", TYPE_INT));
            else {
                char* value = generate_expr(arg);
                appendTAC(CreateTAC("print", value, "", "", get_expr_type(arg)));
            }

            arg = arg->next;
        }
    }
}

void generate_stmt(Node* node)
{
    if (node == NULL)
        return;

    switch (node->type)
    {
        case NODE_DECL:
        case NODE_ARRAY_DECL:
        case NODE_PARAM:
        case NODE_ARG:
            break;

        case NODE_DECL_ASSN:
            if (node->left != NULL)
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

        case NODE_FUNC_DEF:
            generate_function_def(node);
            break;

        case NODE_MAIN:
            generate_main(node);
            break;

        case NODE_RETURN:
            generate_return(node);
            break;

        case NODE_FUNC_CALL:
            generate_expr(node);
            break;

        default:
            generate_expr(node);
            break;
    }
}

void generate_stmt_list(Node* node)
{
    Node* curr = node;
    while (curr != NULL)
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

    while (temp != NULL)
    {
        if (strcmp(temp->result, "if") == 0 && strcmp(temp->op, "goto") == 0)
            printf("if %s goto %s\n", temp->arg1, temp->arg2);
        else if (strcmp(temp->result, "goto") == 0)
            printf("goto %s\n", temp->arg1);
        else if (strcmp(temp->op, "label") == 0)
            printf("%s:\n", temp->result);
        else if (strcmp(temp->op, "func") == 0)
            printf("func %s\n", temp->result);
        else if (strcmp(temp->op, "endfunc") == 0)
            printf("endfunc %s\n", temp->result);
        else if (strcmp(temp->result, "param") == 0)
            printf("param %s\n", temp->arg1);
        else if (strcmp(temp->op, "getparam") == 0)
            printf("%s = getparam %s\n", temp->result, temp->arg1);
        else if (strcmp(temp->op, "call") == 0)
            printf("%s = call %s, %s\n", temp->result, temp->arg1, temp->arg2);
        else if (strcmp(temp->result, "return") == 0) {
            if (temp->arg1[0] != '\0')
                printf("return %s\n", temp->arg1);
            else
                printf("return\n");
        }
        else if (strcmp(temp->result, "print") == 0)
            printf("print %s\n", temp->arg1);
        else if (strcmp(temp->result, "input") == 0)
            printf("input %s\n", temp->arg1);
        else if (strcmp(temp->op, "!") == 0)
            printf("%s = ! %s\n", temp->result, temp->arg1);
        else if (strcmp(temp->op, "=") == 0)
            printf("%s = %s\n", temp->result, temp->arg1);
        else if (strcmp(temp->op, "=[]") == 0)
            printf("%s = %s[%s]\n", temp->result, temp->arg1, temp->arg2);
        else if (strcmp(temp->op, "[]=") == 0)
            printf("%s[%s] = %s\n", temp->result, temp->arg1, temp->arg2);
        else
            printf("%s = %s %s %s\n", temp->result, temp->arg1, temp->op, temp->arg2);

        temp = temp->next;
    }

    printf("===============================\n");
}