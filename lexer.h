#ifndef LEXER_H
#define LEXER_H
#include "lexer.h"

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
    ID      // identifiers
}Tokentype;

typedef struct {
    char lexeme[50];
    Tokentype type;
}Token;

extern Token **tokens;
extern int tokencount ;
extern int capacity ;
void initTokens();

void GetNextToken(FILE *fp);
#endif