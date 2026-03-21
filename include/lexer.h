#ifndef LEXER_H
#define LEXER_H

#include<stdio.h>
typedef enum {
    EOI,    //End of Input
    SEMI,   // ; 
    ADD,    // +
    SUB,    // - 
    MUL,    // *
    DIV,    // /
    MOD,    // %
    LP,     // (
    RP,     // )
    ASSIGN, // =
    LT,     // <
    GT,     // >
    LE,     // <=
    GE,     // >=
    EQ,     // ==
    NE,     // !=
    AND,    // &
    OR,     // ||
    NOT,    // !
    NUM,    // numbers
    ID,     // identifier
    INT,    // Interger
    FLOAT,  // floating point
    START,  // START
    END ,   // END 
    IF,     // if (keyword)
    ELSE,   // else (keyword)
    ELIF,   // elif (keyword)
    WHILE,  // while (keyword)
}Tokentype;

typedef struct {
    char lexeme[64];
    Tokentype type;
}Token;

extern Token **tokens;
extern int tokencount ;
extern int capacity ;
void initTokens();

void GetNextToken(FILE *fp);
#endif