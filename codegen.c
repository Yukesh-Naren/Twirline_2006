#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "codegen.h"
#include "tac.h"
#include "semantics.h"

#define MAX_VARS 500

extern TAC* tacHead;
extern TAC* tacTail;

static char vars[MAX_VARS][50];
static int varCount = 0;

static char strings[MAX_VARS][50];
static int stringCount = 0;

static char floatConsts[MAX_VARS][50];
static int floatConstCount = 0;

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
        if (s[i] == '.') {
            dot++;
        } else if (isdigit((unsigned char)s[i])) {
            digit = 1;
        } else {
            return 0;
        }
    }

    return (dot == 1 && digit);
}

int is_char_literal(const char* s) {
    return s && s[0] == '\'' && s[1] != '\0' && s[2] == '\'' && s[3] == '\0';
}

int get_temp_type(const char* name) {
    TAC* temp = tacHead;

    while (temp != NULL) {
        if (strcmp(temp->result, name) == 0) {
            return temp->type;
        }
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
    if (idx != -1)
        return idx;

    if (floatConstCount >= MAX_VARS) {
        printf("Too many float constants\n");
        exit(1);
    }

    strcpy(floatConsts[floatConstCount], s);
    return floatConstCount++;
}

void load_int_operand(const char* op, const char* reg) {
    if (is_int_literal(op)) {
        printf("    li %s, %s\n", reg, op);
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
    else if (strncmp(op, "tmp", 3) == 0) {
        if (get_temp_type(op) == TYPE_FLOAT)
            load_float_operand(op, freg);
        else {
            printf("    la t3, %s\n", op);
            printf("    lw %s, 0(t3)\n", treg);
            printf("    fcvt.s.w %s, %s\n", freg, treg);
        }
    }
    else if (get_symbol_type(op) == TYPE_FLOAT) {
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
    if (is_int_literal(op)) return 0;

    if (strncmp(op, "tmp", 3) == 0)
        return get_temp_type(op) == TYPE_FLOAT;

    return get_symbol_type(op) == TYPE_FLOAT;
}

int exists_var(const char* name) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(vars[i], name) == 0) return 1;
    }
    return 0;
}

void add_var(const char* name) {
    if (name == NULL || name[0] == '\0') return;
    if (is_int_literal(name)) return;
    if (is_float_literal(name)) return;
    if (is_char_literal(name)) return ; 
    if (strcmp(name, "if") == 0) return;
    if (strcmp(name, "goto") == 0) return;
    if (strcmp(name, "input") == 0) return;
    if (strcmp(name, "print") == 0) return;
    if (exists_var(name)) return;

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

    strcpy(strings[stringCount], s);
    return stringCount++;
}



void collect_variables() {
    TAC* temp = tacHead;

    while (temp != NULL) {
        if (strcmp(temp->result, "if") == 0 && strcmp(temp->op, "goto") == 0) {
            add_var(temp->arg1);
        }
        else if (strcmp(temp->result, "goto") == 0) {
        }
        else if (strcmp(temp->op, "label") == 0) {
        }
        else if (strcmp(temp->result, "print") == 0) {
            if (is_float_literal(temp->arg1))
                add_float_const(temp->arg1);
            else if (!is_int_literal(temp->arg1) && !is_float_literal(temp->arg1) && !exists_var(temp->arg1))
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

    if (strcmp(temp->op, "=") == 0) {
        if (temp->type == TYPE_FLOAT) {
            load_as_float(temp->arg1, "ft0", "t0");
            printf("    la t3, %s\n", temp->result);
            printf("    fsw ft0, 0(t3)\n");
            return;
        }else if (temp->type == TYPE_CHAR){
            load_char_operand(temp->arg1,"t0");
            printf("    la t3, %s\n", temp->result);
            printf("    sb t0, 0(t3)\n");
            return;
        } 
        else {
            load_int_operand(temp->arg1, "t0");
            printf("    la t3, %s\n", temp->result);
            printf("    sw t0, 0(t3)\n");
            return;
        }
    }

    if (strcmp(temp->result, "input") == 0) {
        if (temp->type == TYPE_FLOAT) {
            printf("    li a7, 6\n");
            printf("    ecall\n");
            printf("    la t3, %s\n", temp->arg1);
            printf("    fsw fa0, 0(t3)\n");
        }
        if (temp->type == TYPE_CHAR) {
            printf("    li a7, 12\n");
            printf("    ecall\n");
            printf("    la t3, %s\n", temp->arg1);
            printf("    sb a0, 0(t3)\n");
            return;
        }
        else {
            printf("    li a7, 5\n");
            printf("    ecall\n");
            printf("    la t3, %s\n", temp->arg1);
            printf("    sw a0, 0(t3)\n");
        }
        return;
    }

    if (strcmp(temp->result, "print") == 0) {
        int idx = exists_string(temp->arg1);

        if (idx != -1) {
            printf("    la a0, str%d\n", idx);
            printf("    li a7, 4\n");
            printf("    ecall\n");
        }
        else if (temp->type == TYPE_FLOAT || is_float_literal(temp->arg1)) {
            load_float_operand(temp->arg1, "fa0");
            printf("    li a7, 2\n");
            printf("    ecall\n");
        }
        else if (temp->type == TYPE_CHAR || is_char_literal(temp->arg1)){
            load_char_operand(temp->arg1, "a0");
            printf("    li a7, 11\n");
            printf("    ecall\n");
        }
        else {
            load_int_operand(temp->arg1, "a0");
            printf("    li a7, 1\n");
            printf("    ecall\n");
        }
        return;
    }

    if (strcmp(temp->op, "!") == 0) {
        load_int_operand(temp->arg1, "t0");
        printf("    seqz t1, t0\n");
        printf("    la t3, %s\n", temp->result);
        printf("    sw t1, 0(t3)\n");
        return;
    }

    int operandsAreFloat = operand_is_float(temp->arg1) || operand_is_float(temp->arg2);

    if (operandsAreFloat) {
    load_as_float(temp->arg1, "ft0", "t0");
    load_as_float(temp->arg2, "ft1", "t1");

    if (strcmp(temp->op, "+") == 0) {
        printf("    fadd.s ft2, ft0, ft1\n");
        printf("    la t3, %s\n", temp->result);
        printf("    fsw ft2, 0(t3)\n");
    }
    else if (strcmp(temp->op, "-") == 0) {
        printf("    fsub.s ft2, ft0, ft1\n");
        printf("    la t3, %s\n", temp->result);
        printf("    fsw ft2, 0(t3)\n");
    }
    else if (strcmp(temp->op, "*") == 0) {
        printf("    fmul.s ft2, ft0, ft1\n");
        printf("    la t3, %s\n", temp->result);
        printf("    fsw ft2, 0(t3)\n");
    }
    else if (strcmp(temp->op, "/") == 0) {
        printf("    fdiv.s ft2, ft0, ft1\n");
        printf("    la t3, %s\n", temp->result);
        printf("    fsw ft2, 0(t3)\n");
    }
    else if (strcmp(temp->op, "<") == 0) {
        printf("    flt.s t2, ft0, ft1\n");
        printf("    la t3, %s\n", temp->result);
        printf("    sw t2, 0(t3)\n");
    }
    else if (strcmp(temp->op, ">") == 0) {
        printf("    flt.s t2, ft1, ft0\n");
        printf("    la t3, %s\n", temp->result);
        printf("    sw t2, 0(t3)\n");
    }
    else if (strcmp(temp->op, "<=") == 0) {
        printf("    fle.s t2, ft0, ft1\n");
        printf("    la t3, %s\n", temp->result);
        printf("    sw t2, 0(t3)\n");
    }
    else if (strcmp(temp->op, ">=") == 0) {
        printf("    fle.s t2, ft1, ft0\n");
        printf("    la t3, %s\n", temp->result);
        printf("    sw t2, 0(t3)\n");
    }
    else if (strcmp(temp->op, "==") == 0) {
        printf("    feq.s t2, ft0, ft1\n");
        printf("    la t3, %s\n", temp->result);
        printf("    sw t2, 0(t3)\n");
    }
    else if (strcmp(temp->op, "!=") == 0) {
        printf("    feq.s t2, ft0, ft1\n");
        printf("    xori t2, t2, 1\n");
        printf("    la t3, %s\n", temp->result);
        printf("    sw t2, 0(t3)\n");
    }
    else {
        printf("    # Unknown float operator %s\n", temp->op);
        return;
        }
    }
    else {
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
            printf("    # Unknown integer operator %s\n", temp->op);
            return;
        }

        printf("    la t3, %s\n", temp->result);
        printf("    sw t2, 0(t3)\n");
        return;
    }
}

void generate_riscv_code() {
    TAC* temp;

    varCount = 0;
    stringCount = 0;
    floatConstCount = 0;
    collect_variables();

    printf("\n========= RISC-V CODE =======\n");
    printf(".data\n");

    for (int i = 0; i < stringCount; i++) {
        printf("str%d: .asciz \"%s\"\n", i, strings[i]);
    }

    for (int i = 0; i < floatConstCount; i++) {
        printf("fconst%d: .float %s\n", i, floatConsts[i]);
    }

    for (int i = 0; i < varCount; i++) {
        if (strncmp(vars[i], "tmp", 3) == 0) {
            if (get_temp_type(vars[i]) == TYPE_FLOAT)
                printf("%s: .float 0.0\n", vars[i]);
            else if (get_temp_type(vars[i]) == TYPE_CHAR)
                printf("%s: .byte 0\n", vars[i]);
            else
                printf("%s: .word 0\n", vars[i]);
        } else {
            if (get_symbol_type(vars[i]) == TYPE_FLOAT)
                printf("%s: .float 0.0\n", vars[i]);
            else if (get_temp_type(vars[i]) == TYPE_CHAR)
                printf("%s: .byte 0\n", vars[i]);
            else
                printf("%s: .word 0\n", vars[i]);
        }
    }

    printf("\n.text\n");
    printf("main:\n");

    temp = tacHead;
    while (temp != NULL) {
        generate_riscv_instruction(temp);
        temp = temp->next;
    }

    printf("    li a7, 10\n");
    printf("    ecall\n");
    printf("=============================\n");
}