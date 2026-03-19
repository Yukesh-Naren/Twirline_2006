#include "include/lexer.h"
#include "include/parser.h"
#include<stdio.h>
#include<stdlib.h>

int current = 0;
Token* current_token;

Node* parse(){
    current = 0;
    current_token = tokens[current];

    return parse_program();
}
void advance(){
    if(current < tokencount)
    current++;
    if(current < tokencount)
    current_token = tokens[current];
}

int  match (int expected_type){
    if(current_token->type == expected_type){
        advance();
        return 1;
    } else {
        printf("Syntax Error: Unexpected token '%s'\n",current_token->lexeme);
        exit(1);
    }
}

Node* parse_fact(){
    if(current_token->type == ID || current_token->type == NUM){
        Node* node =CreateNode(current_token->lexeme);
        advance();
        return node;
    }

    if(match(LP)){
        Node* node = parse_assign();
        match(RP);
        return node;
    }

    printf("Syntax Error: Expected Number or Identifier");
    exit(1);
}
Node* parse_term(){
    Node* left = parse_fact();

    while(current_token->type == DIV || current_token->type == MUL || current_token->type == MOD){
        char *op = current_token->lexeme;
        advance();

        Node* right = parse_fact();
        Node* node = CreateNode(op);
        node->left = left;
        node->right = right;

        left=node;
    }
    return left;
}

Node* parse_expr(){
    Node* left = parse_term();

    while(current_token->type == ADD || current_token->type == SUB){
        char *op = current_token->lexeme;
        advance();

        Node* right = parse_term();
        Node* node = CreateNode(op);
        node->left = left;
        node->right = right;

        left = node;
    }
    return left;
}

Node* parse_equal(){
    Node* left = parse_relation();

    while(current_token->type == EQ){
        advance();

        Node* right = parse_relation();
        Node* node = CreateNode("==");
        node->left = left;
        node->right = right;

        left = node;
    }
    return left;
}

Node* parse_relation(){
    Node* left = parse_expr();
    while(current_token->type == LT || current_token->type == GT || current_token->type == LE || current_token->type == GE ){
        
        char *op = current_token->lexeme;
        advance();
        Node* right =parse_expr();

        Node* node = CreateNode(op);
        node->left = left;
        node->right = right;

        left = node;
    }
    return left;
}

Node* parse_assign(){
    if(current_token->type == ID){
        Token* temp = current_token;
        printf("Parsing Assignment...\n");
        if((current+1)<tokencount && tokens[current+1]->type == ASSIGN){
            advance();
            advance();

            Node* right =parse_assign();

            Node* node = CreateNode("=");
            node->left = CreateNode(temp->lexeme);
            node->right = right;

            return node;
        }
    }

    return parse_equal();
}

Node* parse_statement(){
    Node* stmt = parse_assign();

    if(current_token->type != SEMI){
        printf("Syntax Error: Expected ';'\n");
        exit(1);
    }

    advance();

    return stmt;
}

Node* parse_program(){
    Node* head =NULL;
    Node* temp = NULL;

    printf("Parsing statement...\n");
    while(current_token->type !=EOI){
        Node* stmt = parse_statement();

        if(head == NULL){
            head = stmt;
            temp = stmt;
        } else{
            temp->next = stmt;
            temp =stmt;
        }
        
    }
    return head;
}