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
    if (strcmp(temp->op,"!") == 0)
    {
        printf("    lw t0, %s\n",temp->arg1);
        printf("    seqz t1, t0\n");
        printf("    sw t1, %s\n",temp->result);
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
    else if(strcmp(temp->op,"<=") == 0){
        printf("    sle t2, t0, t1\n");
        printf("    sw t2, %s\n", temp->result);
    }
    else if(strcmp(temp->op,">=") == 0){
        printf("    sge t2, t0, t1\n");
        printf("    sw t2, %s\n", temp->result);
    }
    else if(strcmp(temp->op,"<") == 0){
        printf("    slt t2, t0, t1\n");
        printf("    sw t2, %s\n", temp->result);
    }
    else if(strcmp(temp->op,">") == 0){
        printf("    sgt t2, t0, t1\n");
        printf("    sw t2, %s\n", temp->result);
    }
    else if(strcmp(temp->op,"==") == 0){
        printf("    sub t2, t0, t1\n");
        printf("    seqz t2, t2\n");
        printf("    sw t2, %s\n", temp->result);
    }
    else if(strcmp(temp->op,"!=") == 0){
        printf("    sle t2, t0, t1\n");
        printf("    snez t2, t2\n");
        printf("    sw t2, %s\n", temp->result);
    }
    else if(strcmp(temp->op,"&&") == 0){
        printf("    snez t0, t0\n");
        printf("    snez t1, t1\n", temp->result);
        printf("    and t2, t0, t1\n");
        printf("    sw t2, %s\n", temp->result);
    }
    else if(strcmp(temp->op,"||") == 0){
        printf("    snez t0, t0\n");
        printf("    snez t1, t1\n", temp->result);
        printf("    or t2, t0, t1\n");
        printf("    sw t2, %s\n", temp->result);
    }
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