#ifndef PARSER_H
#define PARSER_H

#include<lexer.h>
#include"parser.h"

extern Token *currentToken;

void advance();
void match();
void error();

void E();
void Eprime();

void T();
void Tprime();
void F();

#endif