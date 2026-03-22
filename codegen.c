#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "codegen.h"
#include "tac.h"

#define MAX_VARS 500

extern TAC* tacHead;
extern TAC* tacTail;

static char vars[MAX_VARS][50];
static int varCount = 0;

static char strings[MAX_VARS][50];
static int stringCount = 0;

/* ---------------- HELPERS ---------------- */

int is_number(const char* s) {
    if (s == NULL || *s == '\0') return 0;

    int i = 0;
    if (s[0] == '-') i++;

    for (; s[i]; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

int exists_var(const char* name) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(vars[i], name) == 0) return 1;
    }
    return 0;
}

void add_var(const char* name) {
    if (name == NULL || name[0] == '\0') return;
    if (is_number(name)) return;
    if (strcmp(name, "if") == 0) return;
    if (strcmp(name, "goto") == 0) return;
    if (exists_var(name)) return;

    strcpy(vars[varCount++], name);
}

void collect_variables() {
    TAC* temp = tacHead;

    while (temp != NULL) {
        // if cond goto L1  
        if (strcmp(temp->result, "if") == 0 && strcmp(temp->op, "goto") == 0) {
            add_var(temp->arg1);
        }
        // goto L1 
        else if (strcmp(temp->result, "goto") == 0) {
            /* nothing */
        }
        // label 
        else if (strcmp(temp->op, "label") == 0) {
            /* nothing */
        }
        //string
        else if (strcmp(temp->result, "print") == 0) {
            if (!is_number(temp->arg1) && !exists_var(temp->arg1))
                add_string(temp->arg1);
        }
        else {
            add_var(temp->result);
            add_var(temp->arg1);
            add_var(temp->arg2);
        }

        temp = temp->next;
    }
}

void load_operand(const char* op, const char* reg) {
    if (is_number(op))
        printf("    li %s, %s\n", reg, op);
    else{
        printf("    la t3, %s\n", op);
        printf("    lw %s, 0(t3)\n", reg);
    }
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

/* ---------------- ONE TAC INSTRUCTION ---------------- */

void generate_riscv_instruction(TAC* temp) {
    if (temp == NULL) return;

    
    // if goto
    if (strcmp(temp->result, "if") == 0 && strcmp(temp->op, "goto") == 0) {
        load_operand(temp->arg1, "t0");
        printf("    bne t0, x0, %s\n", temp->arg2);
        return;
    }

    // goto  
    if (strcmp(temp->result, "goto") == 0) {
        printf("    j %s\n", temp->arg1);
        return;
    }

    // label
    if (strcmp(temp->op, "label") == 0) {
        printf("%s:\n", temp->result);
        return;
    }

    // assignment 
    if (strcmp(temp->op, "=") == 0) {
        load_operand(temp->arg1, "t0");
        printf("    la t3 ,%s\n", temp->result);
        printf("    sw t0, 0(t3)\n");
        return;
    }
    //input
    if(strcmp(temp->result,"input") == 0){
        printf("    li a7,5\n");
        printf("    ecall\n");
        printf("    la t3,%s\n",temp->arg1);
        printf("    sw a0, 0(t3)\n");
        return;
    }
    //print
    if (strcmp(temp->result, "print") == 0) {

        int idx = exists_string(temp->arg1);
        // STRING
        if (idx != -1) {
            printf("    la a0, str%d\n", idx);   
            printf("    li a7, 4\n");
            printf("    ecall\n");
        }
        else {
            // NUMBER or VARIABLE
            load_operand(temp->arg1, "a0");
            printf("    li a7, 1\n");
            printf("    ecall\n");
        }

        return;
    }
    // unary not 
    if (strcmp(temp->op, "!") == 0) {
        load_operand(temp->arg1, "t0");
        printf("    seqz t1, t0\n");
        printf("    la t3 ,%s\n", temp->result);
        printf("    sw t1, 0(t3)\n");
        return;
    }

    // binary ops 
    load_operand(temp->arg1, "t0");
    load_operand(temp->arg2, "t1");

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
        printf("    # Unknown operator %s\n", temp->op);
        return;
    }

    printf("    la t3 ,%s\n", temp->result);
    printf("    sw t2, 0(t3)\n");
}

//Code Generation

void generate_riscv_code() {
    TAC* temp;

    varCount = 0;
    collect_variables();

    printf("\n========= RISC-V CODE =======\n");
    printf(".data\n");

    for (int i = 0; i < stringCount; i++) {
    printf("str%d: .asciz \"%s\"\n", i, strings[i]);
    }

    for (int i = 0; i < varCount; i++) {
        printf("%s: .word 0\n", vars[i]);
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