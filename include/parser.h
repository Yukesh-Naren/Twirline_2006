#ifndef PARSER_H
#define PARSER_H

#include"lexer.h"
#include"AST.h"

void advance();
int match (int expected_type);
Node* parse();
Node* parse_program();
Node* parse_statement();
Node* parse_stmt_list();
Node* parse_expr();
Node* parse_additive();
Node* parse_term();
Node* parse_fact();
Node* parse_assign();
Node* parse_relation();
Node* parse_equal();
Node* parse_logical_and();
Node* parse_logical_or();
Node* parse_declaration();
Node* parse_declaration_assignment(char* var_name);
Node* parse_if();
Node* parse_while();
Node* parse_block();
<<<<<<< HEAD
Node* parse_print();
Node* parse_input_stmt();
=======
Node* parse_while();
>>>>>>> c0e08d59c24a4fefe4ebbd400716c562c0782a35
#endif