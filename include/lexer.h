#ifndef LEXER_H
#define LEXER_H

#include<stdio.h>
typedef enum {
    EOI,    //End of Input0
    SEMI,   // ; 1
    ADD,    // + 2
    SUB,    // - 3
    MUL,    // * 4
    DIV,    // / 5
    MOD,    // % 6
    LP,     // ( 7
    RP,     // ) 8
    ASSIGN, // = 9
    LT,     // < 10
    GT,     // > 11
    LE,     // <= 12
    GE,     // >= 13
    EQ,     // == 14
    NE,     // != 15
    AND,    // & 16
    OR,     // || 17
    NOT,    // ! 18
    NUM_INT,   // integer 19
    NUM_FLOAT, // float point 20
    ID,     // identifier 21
    START,  // START 22
    END ,   // END  23
    IF,     // if (keyword) 24
    ELSE,   // else (keyword) 25
    ELIF,   // elif (keyword) 26
    WHILE,  // while (keyword) 27
    PRINT,  // print 28
    DQ,     // " 29
    STRING, // strings 30
    COMMA,  // , 31 
    INPUT,  // input 32
    INT ,   // int (keyword) 34
    FLOAT,  // float(keyword) 35
    CHAR,   // char(keyword) 36
    CHAR_CONST, // character constant 37 
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