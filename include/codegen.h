#ifndef CODEGEN_H
#define CODEGEN_H

#include "TAC.h"

void generate_riscv_code();
void generate_riscv_instruction(TAC* temp);
int add_string(const char *s);
void load_operand(const char* op, const char* reg);
int exists_string(const char* s);
#endif