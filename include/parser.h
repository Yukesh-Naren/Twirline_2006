#ifndef PARSER_H
#define PARSER_H

#include"lexer.h"
#include"AST.h"

void advance();
int match (int expected_type);
Node* parse();
Node* parse_param_list();
Node* parse_arg_list();
Node* parse_function_def();
Node* parse_function_call(char* name);
Node* parse_return_stmt();
Node* parse_array_assignment(char* name);
Node* parse_array_dimensions();
Node* parse_declaration();
Node* parse_fact();
Node* parse_term();
Node* parse_additive();
Node* parse_relation();
Node* parse_equal();
Node* parse_logical_and();
Node* parse_logical_or();
Node* parse_assign();
Node* parse_expr();
Node* parse_print();
Node* parse_input_stmt();
Node* parse_if();
Node* parse_for();
Node* parse_while();
Node* parse_statement();
Node* parse_stmt_list();
Node* parse_main_block();
Node* parse_program();

#endif