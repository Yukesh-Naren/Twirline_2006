#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "codegen.h"
#include "tac.h"
#include "semantics.h"

#define MAX_VARS 500
#define MAX_PARAMS 50

extern TAC* tacHead;
extern FunctionSymbol* functionHead;

static char vars[MAX_VARS][50];
static int varCount = 0;

static char strings[MAX_VARS][200];
static int stringCount = 0;

static char floatConsts[MAX_VARS][50];
static int floatConstCount = 0;

static char pendingParams[MAX_PARAMS][50];
static int pendingParamTypes[MAX_PARAMS];
static int pendingParamCount = 0;

static char currentFunction[50] = "";

static void codegen_error(const char* msg) {
    printf("Codegen Error: %s\n", msg);
    exit(1);
}

static const char* asm_func_label(const char* name) {
    if (strcmp(name, "main") == 0) return "__user_main";
    return name;
}

static void asm_func_end_label(const char* name, char* out) {
    if (strcmp(name, "main") == 0)
        strcpy(out, "__user_main_end");
    else
        sprintf(out, "%s_end", name);
}

int is_int_literal(const char* s) {
    if (s == NULL || *s == '\0') return 0;

    int i = 0;
    if (s[0] == '-') i++;

    if (s[i] == '\0') return 0;

    for (; s[i]; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

int is_float_literal(const char* s) {
    if (s == NULL || *s == '\0') return 0;

    int i = 0, dot = 0, digit = 0;
    if (s[0] == '-') i++;

    for (; s[i]; i++) {
        if (s[i] == '.') dot++;
        else if (isdigit((unsigned char)s[i])) digit = 1;
        else return 0;
    }

    return (dot == 1 && digit);
}

int is_char_literal(const char* s) {
    return s && s[0] == '\'' && s[1] != '\0' && s[2] == '\'' && s[3] == '\0';
}

int get_temp_type(const char* name) {
    TAC* temp = tacHead;

    while (temp != NULL) {
        if (strcmp(temp->result, name) == 0)
            return temp->type;
        temp = temp->next;
    }

    return TYPE_INT;
}

int exists_float_const(const char* s) {
    for (int i = 0; i < floatConstCount; i++) {
        if (strcmp(floatConsts[i], s) == 0)
            return i;
    }
    return -1;
}

int add_float_const(const char* s) {
    int idx = exists_float_const(s);
    if (idx != -1) return idx;

    if (floatConstCount >= MAX_VARS)
        codegen_error("too many float constants");

    strcpy(floatConsts[floatConstCount], s);
    return floatConstCount++;
}

static int is_array_param_name_codegen(const char* name) {
    FunctionSymbol* fn = functionHead;

    while (fn != NULL) {
        Node* p = fn->params;
        while (p != NULL) {
            if (p->type == NODE_PARAM && p->left != NULL) {
                if (strcmp(p->left->val, name) == 0)
                    return (p->left->right != NULL);
            }
            p = p->next;
        }
        fn = fn->next;
    }

    return 0;
}

static int is_array_like_name_codegen(const char* name) {
    Symbol* sym;

    if (name == NULL) return 0;

    sym = lookup_symbol((char*)name);
    if (sym != NULL && sym->is_array)
        return 1;

    if (is_array_param_name_codegen(name))
        return 1;

    return 0;
}

int get_identifier_type_codegen(const char* name) {
    if (name == NULL) return TYPE_INT;

    if (strncmp(name, "tmp", 3) == 0)
        return get_temp_type(name);

    {
        Symbol* sym = lookup_symbol((char*)name);
        if (sym != NULL)
            return sym->type;
    }

    return TYPE_INT;
}

void load_int_operand(const char* op, const char* reg) {
    if (is_int_literal(op)) {
        printf("    li %s, %s\n", reg, op);
    } else if (is_char_literal(op)) {
        printf("    li %s, %d\n", reg, op[1]);
    } else {
        printf("    la t3, %s\n", op);
        printf("    lw %s, 0(t3)\n", reg);
    }
}

void load_float_operand(const char* op, const char* reg) {
    int idx = exists_float_const(op);

    if (idx != -1) {
        printf("    la t3, fconst%d\n", idx);
        printf("    flw %s, 0(t3)\n", reg);
    } else {
        printf("    la t3, %s\n", op);
        printf("    flw %s, 0(t3)\n", reg);
    }
}

void load_as_float(const char* op, const char* freg, const char* treg) {
    if (is_float_literal(op)) {
        load_float_operand(op, freg);
    }
    else if (is_int_literal(op)) {
        printf("    li %s, %s\n", treg, op);
        printf("    fcvt.s.w %s, %s\n", freg, treg);
    }
    else if (is_char_literal(op)) {
        printf("    li %s, %d\n", treg, op[1]);
        printf("    fcvt.s.w %s, %s\n", freg, treg);
    }
    else if (get_identifier_type_codegen(op) == TYPE_FLOAT) {
        load_float_operand(op, freg);
    }
    else {
        printf("    la t3, %s\n", op);
        printf("    lw %s, 0(t3)\n", treg);
        printf("    fcvt.s.w %s, %s\n", freg, treg);
    }
}

void load_char_operand(const char* op, const char* reg) {
    if (is_char_literal(op)) {
        printf("    li %s, %d\n", reg, op[1]);
    } else {
        printf("    la t3, %s\n", op);
        printf("    lb %s, 0(t3)\n", reg);
    }
}

int operand_is_float(const char* op) {
    if (op == NULL || op[0] == '\0') return 0;
    if (is_float_literal(op)) return 1;
    if (is_int_literal(op) || is_char_literal(op)) return 0;
    return get_identifier_type_codegen(op) == TYPE_FLOAT;
}

int exists_var(const char* name) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(vars[i], name) == 0) return 1;
    }
    return 0;
}

int is_label_name(const char *s) {
    if (!s || s[0] != 'L') return 0;

    for (int i = 1; s[i]; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

void add_var(const char* name) {
    if (name == NULL || name[0] == '\0') return;
    if (is_int_literal(name)) return;
    if (is_float_literal(name)) return;
    if (is_char_literal(name)) return;
    if (is_label_name(name)) return;
    if (strcmp(name, "if") == 0) return;
    if (strcmp(name, "goto") == 0) return;
    if (strcmp(name, "input") == 0) return;
    if (strcmp(name, "print") == 0) return;
    if (strcmp(name, "param") == 0) return;
    if (strcmp(name, "return") == 0) return;
    if (exists_var(name)) return;

    if (varCount >= MAX_VARS)
        codegen_error("too many variables");

    strcpy(vars[varCount++], name);
}

int exists_string(const char* s) {
    for (int i = 0; i < stringCount; i++) {
        if (strcmp(strings[i], s) == 0) return i;
    }
    return -1;
}

int add_string(const char* s) {
    int idx = exists_string(s);
    if (idx != -1) return idx;

    if (stringCount >= MAX_VARS)
        codegen_error("too many strings");

    strcpy(strings[stringCount], s);
    return stringCount++;
}

int get_array_total_size(Symbol* sym) {
    int total = 1;
    for (int i = 0; i < sym->dimensions; i++)
        total *= sym->dim_sizes[i];
    return total;
}

int get_array_elem_size(Symbol* sym) {
    if (sym->type == TYPE_CHAR) return 1;
    return 4;
}

void add_function_params_as_vars() {
    FunctionSymbol* fn = functionHead;

    while (fn != NULL) {
        Node* p = fn->params;
        while (p != NULL) {
            if (p->left) add_var(p->left->val);
            p = p->next;
        }
        fn = fn->next;
    }
}

void collect_variables() {
    TAC* temp = tacHead;

    add_function_params_as_vars();

    while (temp != NULL) {
        if (strcmp(temp->result, "if") == 0 && strcmp(temp->op, "goto") == 0) {
            add_var(temp->arg1);
        }

        else if (strcmp(temp->op, "label") == 0) {

        }
        else if (strcmp(temp->result, "goto") == 0) {
            
        }
        else if (strcmp(temp->op, "func") == 0 || strcmp(temp->op, "endfunc") == 0) {
        }
        else if (strcmp(temp->result, "param") == 0) {
            if (is_float_literal(temp->arg1)) add_float_const(temp->arg1);
            else add_var(temp->arg1);
        }
        else if (strcmp(temp->op, "call") == 0) {
            add_var(temp->result);
        }
        else if (strcmp(temp->result, "return") == 0) {
            if (is_float_literal(temp->arg1)) add_float_const(temp->arg1);
            else add_var(temp->arg1);
        }
        else if (strcmp(temp->result, "print") == 0) {
            if (is_float_literal(temp->arg1)) add_float_const(temp->arg1);
            else if (!is_int_literal(temp->arg1) &&
                     !is_float_literal(temp->arg1) &&
                     !is_char_literal(temp->arg1) &&
                     !exists_var(temp->arg1))
                add_string(temp->arg1);
        }
        else if (strcmp(temp->result, "input") == 0) {
            add_var(temp->arg1);
        }
        else {
            if (is_float_literal(temp->result)) add_float_const(temp->result);
            else add_var(temp->result);

            if (is_float_literal(temp->arg1)) add_float_const(temp->arg1);
            else add_var(temp->arg1);

            if (is_float_literal(temp->arg2)) add_float_const(temp->arg2);
            else add_var(temp->arg2);
        }

        temp = temp->next;
    }
}

static void load_address_of_array_like(const char* name, const char* reg) {
    Symbol* sym = lookup_symbol((char*)name);

    if (sym != NULL && sym->is_array && !is_array_param_name_codegen(name)) {
        printf("    la %s, %s\n", reg, name);
        return;
    }

    if (is_array_param_name_codegen(name)) {
        printf("    la t3, %s\n", name);
        printf("    lw %s, 0(t3)\n", reg);
        return;
    }

    printf("    la %s, %s\n", reg, name);
}

static void emit_store_word(const char* name, const char* reg) {
    printf("    la t3, %s\n", name);
    printf("    sw %s, 0(t3)\n", reg);
}

void generate_riscv_instruction(TAC* temp) {
    if (temp == NULL) return;

    if (strcmp(temp->result, "if") == 0 && strcmp(temp->op, "goto") == 0) {
        load_int_operand(temp->arg1, "t0");
        printf("    bne t0, x0, %s\n", temp->arg2);
        return;
    }

    if (strcmp(temp->result, "goto") == 0) {
        printf("    j %s\n", temp->arg1);
        return;
    }

    if (strcmp(temp->op, "label") == 0) {
        printf("%s:\n", temp->result);
        return;
    }

    if (strcmp(temp->op, "func") == 0) {
        strcpy(currentFunction, temp->result);
        printf("%s:\n", asm_func_label(temp->result));
        printf("    addi sp, sp, -16\n");
        printf("    sw ra, 12(sp)\n");
        return;
    }

    if (strcmp(temp->op, "endfunc") == 0) {
        char endLabel[80];
        asm_func_end_label(temp->result, endLabel);
        printf("%s:\n", endLabel);
        printf("    lw ra, 12(sp)\n");
        printf("    addi sp, sp, 16\n");
        printf("    ret\n");
        currentFunction[0] = '\0';
        return;
    }

    if (strcmp(temp->op, "getparam") == 0) {
        int idx = atoi(temp->arg1);
        char areg[8];
        sprintf(areg, "a%d", idx);
        emit_store_word(temp->result, areg);
        return;
    }

    if (strcmp(temp->result, "param") == 0) {
        if (pendingParamCount >= MAX_PARAMS)
            codegen_error("too many params");

        strcpy(pendingParams[pendingParamCount], temp->arg1);
        pendingParamTypes[pendingParamCount] = temp->type;
        pendingParamCount++;
        return;
    }

    if (strcmp(temp->op, "call") == 0) {
        for (int i = 0; i < pendingParamCount; i++) {
            if (is_array_like_name_codegen(pendingParams[i])) {
                Symbol* sym = lookup_symbol((char*)pendingParams[i]);

                if (sym != NULL && sym->is_array && !is_array_param_name_codegen(pendingParams[i])) {
                    printf("    la t0, %s\n", pendingParams[i]);
                }
                else if (is_array_param_name_codegen(pendingParams[i])) {
                    printf("    la t3, %s\n", pendingParams[i]);
                    printf("    lw t0, 0(t3)\n");
                }
                else {
                    printf("    la t0, %s\n", pendingParams[i]);
                }

                printf("    mv a%d, t0\n", i);
            } else {
                load_int_operand(pendingParams[i], "t0");
                printf("    mv a%d, t0\n", i);
            }
        }

        pendingParamCount = 0;
        printf("    jal ra, %s\n", asm_func_label(temp->arg1));
        emit_store_word(temp->result, "a0");
        return;
    }

    if (strcmp(temp->result, "return") == 0) {
        char endLabel[80];
        if (temp->arg1[0] != '\0') {
            load_int_operand(temp->arg1, "t0");
            printf("    mv a0, t0\n");
        }
        asm_func_end_label(currentFunction, endLabel);
        printf("    j %s\n", endLabel);
        return;
    }

    if (strcmp(temp->op, "=") == 0) {
        load_int_operand(temp->arg1, "t0");
        emit_store_word(temp->result, "t0");
        return;
    }

    if (strcmp(temp->op, "=[]") == 0) {
        load_int_operand(temp->arg2, "t0");
        printf("    li t1, 4\n");
        printf("    mul t2, t0, t1\n");
        load_address_of_array_like(temp->arg1, "t3");
        printf("    add t4, t3, t2\n");
        printf("    lw t5, 0(t4)\n");
        emit_store_word(temp->result, "t5");
        return;
    }

    if (strcmp(temp->op, "[]=") == 0) {
        load_int_operand(temp->arg1, "t0");
        printf("    li t1, 4\n");
        printf("    mul t2, t0, t1\n");
        load_address_of_array_like(temp->result, "t3");
        printf("    add t4, t3, t2\n");
        load_int_operand(temp->arg2, "t5");
        printf("    sw t5, 0(t4)\n");
        return;
    }

    if (strcmp(temp->result, "print") == 0) {
        load_int_operand(temp->arg1, "a0");
        printf("    li a7, 1\n");
        printf("    ecall\n");
        return;
    }

    if (strcmp(temp->op, "!") == 0) {
        load_int_operand(temp->arg1, "t0");
        printf("    seqz t1, t0\n");
        emit_store_word(temp->result, "t1");
        return;
    }

    load_int_operand(temp->arg1, "t0");
    load_int_operand(temp->arg2, "t1");

    if (strcmp(temp->op, "+") == 0) {
        printf("    add t2, t0, t1\n");
    }
    else if (strcmp(temp->op, "-") == 0) {
        printf("    sub t2, t0, t1\n");
    }
    else if (strcmp(temp->op, "*") == 0) {
        printf("    mul t2, t0, t1\n");
    }
    else if (strcmp(temp->op, "/") == 0) {
        printf("    div t2, t0, t1\n");
    }
    else if (strcmp(temp->op, "%") == 0) {
        printf("    rem t2, t0, t1\n");
    }
    else if (strcmp(temp->op, "<") == 0) {
        printf("    slt t2, t0, t1\n");
    }
    else if (strcmp(temp->op, ">") == 0) {
        printf("    slt t2, t1, t0\n");
    }
    else if (strcmp(temp->op, "<=") == 0) {
        printf("    slt t2, t1, t0\n");
        printf("    xori t2, t2, 1\n");
    }
    else if (strcmp(temp->op, ">=") == 0) {
        printf("    slt t2, t0, t1\n");
        printf("    xori t2, t2, 1\n");
    }
    else if (strcmp(temp->op, "==") == 0) {
        printf("    sub t2, t0, t1\n");
        printf("    seqz t2, t2\n");
    }
    else if (strcmp(temp->op, "!=") == 0) {
        printf("    sub t2, t0, t1\n");
        printf("    snez t2, t2\n");
    }
    else if (strcmp(temp->op, "&&") == 0) {
        printf("    snez t0, t0\n");
        printf("    snez t1, t1\n");
        printf("    and t2, t0, t1\n");
    }
    else if (strcmp(temp->op, "||") == 0) {
        printf("    snez t0, t0\n");
        printf("    snez t1, t1\n");
        printf("    or t2, t0, t1\n");
    }
    else {
        return;
    }

    emit_store_word(temp->result, "t2");
}

void generate_riscv_code() {
    TAC* temp;
    int hasUserMain = 0;

    varCount = 0;
    stringCount = 0;
    floatConstCount = 0;
    pendingParamCount = 0;
    currentFunction[0] = '\0';

    collect_variables();

    temp = tacHead;
    while (temp != NULL) {
        if (strcmp(temp->op, "func") == 0 && strcmp(temp->result, "main") == 0) {
            hasUserMain = 1;
            break;
        }
        temp = temp->next;
    }

    printf("\n.data\n");

    for (int i = 0; i < floatConstCount; i++) {
        printf("fconst%d: .float %s\n", i, floatConsts[i]);
    }

    for (int i = 0; i < varCount; i++) {
        if (strncmp(vars[i], "tmp", 3) == 0) {
            printf("%s: .word 0\n", vars[i]);
        } else {
            Symbol* sym = lookup_symbol(vars[i]);

            if (is_array_param_name_codegen(vars[i])) {
                printf("%s: .word 0\n", vars[i]);
            }
            else if (sym && sym->is_array) {
                int total = get_array_total_size(sym);
                int elemSize = get_array_elem_size(sym);
                printf("%s: .space %d\n", vars[i], total * elemSize);
            }
            else {
                printf("%s: .word 0\n", vars[i]);
            }
        }
    }

    printf("\n.text\n");
    printf(".globl main\n");
    printf("main:\n");

    if (hasUserMain) {
        printf("    jal ra, __user_main\n");
        printf("    li a7, 10\n");
        printf("    ecall\n");
    }

    temp = tacHead;
    while (temp != NULL) {
        generate_riscv_instruction(temp);
        temp = temp->next;
    }

    if (!hasUserMain) {
        printf("    li a7, 10\n");
        printf("    ecall\n");
    }
}