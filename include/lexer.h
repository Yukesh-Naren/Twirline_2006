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
    NUM,    // numbers
    ID,     // identifier
<<<<<<< HEAD
    INT,     // integer
    FLOAT,
=======
    INT,    // Interger
    FLOAT   // floating point
>>>>>>> bc47f11a1d216f7484fb1230a4a5586689642f57
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