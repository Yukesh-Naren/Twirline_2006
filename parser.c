#include "include/lexer.h"
#include "include/parser.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
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
Node* parse_declaration_assignment(char* var_name){

    Node* expr = parse_expr();

    Node* n = CreateNode("Decl_Assign",NODE_DECL_ASSN);
    n->left = CreateNode("=" , NODE_ASSIGN);
    n->left->left = CreateNode(var_name , NODE_ID);
    n->left->right = expr;
    // n->type = NODE_DECL_ASSN;

    return n;
} // Gets input like x(int) = 5;

Node* parse_declaration(){
    char* var_name = current_token->lexeme;
    match(ID);
    match(LP);
        char* type_name;
    if (current_token->type == INT) {
        type_name = "int";
        match(INT);
    } else if (current_token->type == FLOAT) {
        type_name = "float";
        match(FLOAT);
    } else {
        printf("Syntax Error: Expected 'int' or 'float' but got '%s'\n", current_token->lexeme);
        exit(1);
    }
    match(RP);
    if(current_token->type == SEMI){
        Node* n = CreateNode("Declaration",NODE_DECL);
        n->left = CreateNode(var_name, NODE_ID);
        n->left->left = CreateNode(type_name,NODE_TYPE);
        return n;
    }
    else if(current_token->type == ASSIGN){
        match(ASSIGN);
        Node* n = parse_declaration_assignment(var_name);
        return n;
    }
    printf("Syntax Error: Expected ';' ");
    exit(1);
 }  //Gets input like x(int);

Node* parse_fact(){
    if(current_token->type == ID ){
        Node* node =CreateNode(current_token->lexeme,NODE_ID);
        advance();
        return node;
    }

    if(current_token->type == NUM ){
        Node* node =CreateNode(current_token->lexeme,NODE_CONST);
        advance();
        return node;
    }
    if(current_token->type  == LP){
        match(LP);
        Node* node = parse_expr();
        match(RP);
        return node;
    }

    printf("Syntax Error: Expected Number or Identifier but got '%s'\n",current_token->lexeme);
    exit(1);
}
Node* parse_term(){
    Node* left = parse_fact();

    while(current_token->type == DIV || current_token->type == MUL || current_token->type == MOD){
        char *op = strdup(current_token->lexeme);
        advance();

        Node* right = parse_fact();
        Node* node = CreateNode(op,NODE_OP);
        node->left = left;
        node->right = right;

        left=node;
    }
    return left;
}

Node* parse_expr(){
    Node* left = parse_term();

    while(current_token->type == ADD || current_token->type == SUB){
        char *op = strdup(current_token->lexeme);
        advance();

        Node* right = parse_term();
        Node* node = CreateNode(op,NODE_OP);
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
        Node* node = CreateNode("==",NODE_OP);
        node->left = left;
        node->right = right;

        left = node;
    }
    return left;
}

Node* parse_relation(){
    Node* left = parse_expr();
    while(current_token->type == LT || current_token->type == GT || current_token->type == LE || current_token->type == GE ){
        
        char *op = strdup(current_token->lexeme);
        advance();
        Node* right =parse_expr();

        Node* node = CreateNode(op,NODE_OP);
        node->left = left;
        node->right = right;

        left = node;
    }
    return left;
}

Node* parse_assign(){
    if(current_token->type == ID){
        if((current+1)<tokencount && tokens[current+1]->type == LP){
            return parse_declaration();
        }
        // printf("Parsing Assignment...\n");
        if((current+1)<tokencount && tokens[current+1]->type == ASSIGN){
            char* name = strdup(current_token->lexeme);
            advance();
            match(ASSIGN);

            Node* right =parse_assign();

            Node* node = CreateNode("=",NODE_ASSIGN);
            node->left = CreateNode(name,NODE_ID);
            node->right = right;
//             printf("LEFT: %s\n", node->left ? node->left->val : "NULL");
// printf("RIGHT: %s\n", node->right ? node->right->val : "NULL");
            return node;
        }
    }
    return parse_equal();
}

Node* parse_statement(){
    Node* stmt = parse_assign();

    if(current_token->type != SEMI){
        printf("Error near token: %s\n", current_token->lexeme);
        printf("Syntax Error: Expected ';'\n");
        exit(1);
    }

    advance();

    return stmt;
}

Node* parse_program(){
    Node* head =NULL;
    Node* temp = NULL;

    // printf("Parsing statement...\n");
    while(current_token->type !=EOI){
        if(current_token->type == SEMI){
            advance();
            continue;
        }

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