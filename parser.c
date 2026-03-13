#include "lexer.h"
#include "parser.h"
#include<stdio.h>
#include<stdlib.h>

Token lookahead;
int pos=0;

void advance(){
    lookahead=tokens[++pos];
}

int match(Tokentype ty){
    if(lookahead.type == ty){
        advance();
        return 1;
    }
    else
        printf("Syntax Error\n");
        return 0;
}

void E(Token t){
    if()
    {
        
    }
}