#include <stdio.h>
#include<string.h>
#include"codegen.h"
#include"TAC.h"

extern TAC* tachead;
extern TAC* tacTail;
void generate_riscv_instruction(TAC* temp){
    if(temp == NULL)
    return;

    if(strcmp(temp->op, "=") == 0){
        printf("    lw t0, %s\n",temp->arg1);
        printf("    sw t0, %s\n",temp->result);
        return;
    }

    printf("    lw t0, %s\n",temp->arg1);
    printf("    lw t1, %s\n",temp->arg2);
    
    if(strcmp(temp->op, "+") == 0)
    printf("    add t2, t0, t1\n");
    else if(strcmp(temp->op,"-") == 0)
    printf("    sub t2, t0, t1\n");
    else if(strcmp(temp->op,"*") == 0)
    printf("    mul t2, t0, t1\n");
    else if(strcmp(temp->op,"%") == 0)
    printf("    rem t2, t0, t1\n");
    else
    {
        printf(" # Unknown operator %s \n",temp->op);
        return;
    }

    printf("    sw t2, %s\n",temp->result);
}

void generate_riscv_code(){
    TAC* temp = tacHead;

    printf("\n=========RISC-V CODE=======\n");
    printf(".data\n");

    temp = tacHead;
    while (temp !=NULL){
        printf("%s:.word 0\n",temp->result);
        temp=temp->next;
    }

    printf("\n.text\n");
    printf("main:\n");

    temp = tacHead;
    while (temp != NULL){
        generate_riscv_instruction(temp);
        temp = temp->next;
    }

    printf("    li a7, 10\n");
    printf("    ecall\n");
    printf("=========================\n");
}