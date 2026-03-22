#include<stdio.h>
#include<stdlib.h>
#include "include/lexer.h"
#include<string.h>
#include <ctype.h>

Token **tokens;
int tokencount = 0;
int capacity = 10;
void initTokens()
{
    tokens = malloc(sizeof(Token*) * capacity);
}
Token* CreateToken(Tokentype type, char *lexeme){
    Token *t =(Token*)malloc(sizeof(Token));

    t->type = type;
    strncpy(t->lexeme,lexeme,63);
    t->lexeme[63] = '\0';
    return t;
}

void addToken(Token *t)
{
    if(tokencount >= capacity)
    {
        capacity *=2;
        tokens = realloc(tokens, sizeof(Token*) * capacity);
    }
    
    tokens[tokencount++] = t;

    printf("TOKEN[%d]: '%s'\n", tokencount, t->lexeme);
}


void GetNextToken(FILE* fp)
{
    int ch;

    while((ch = fgetc(fp))!=EOF)
    {
        if(isspace(ch))
        continue;

        if(ch=='+')
        {
            Token *t =CreateToken(ADD, "+");
            addToken(t);       
        }

        else if(ch=='-')
        {
            Token *t =CreateToken(SUB, "-");
            addToken(t);       
        }

        else if(ch=='*')
        {
            Token *t =CreateToken(MUL, "*");
            addToken(t);       
        }

        else if(ch=='/')
        {
            Token *t =CreateToken(DIV, "/");
            addToken(t);       
        }

        else if(ch=='%')
        {
            Token *t =CreateToken(MOD, "%");
            addToken(t);       
        }

        else if(ch=='(')
        {
            Token *t =CreateToken(LP, "(");
            addToken(t);       
        }

        else if( ch == ',')
        {
            Token *t = CreateToken(COMMA,",");
            addToken(t);
        }
        
        else if(ch==')')
        {
            Token *t =CreateToken(RP, ")");
            addToken(t);       
        }

        else if( ch == '"')
        {
            char buffer[50];
            int c=0;
            int j=0;
            ch = fgetc(fp);
            while(ch!='"' && ch != EOF){
                buffer[j++] = ch;
                ch = fgetc(fp);
            }
            
            buffer[j]='\0';
            Token *t2 = CreateToken(STRING, buffer);
            addToken(t2);
        }

        else if(ch == ';')
        {
            Token *t = CreateToken(SEMI,";");
            addToken(t);
        }

        else if(ch == '=')
        {
            int next = fgetc(fp);
            if(next == '=')
            {
                Token *t =CreateToken(EQ, "==");
                addToken(t);
            }
            else
            {
                Token *t=CreateToken(ASSIGN,"=");
                addToken(t);
                ungetc(next,fp);
            }
        }

        else if(ch == '<')
        {
            int next = fgetc(fp);
            if(next == '=')
            {
                Token *t = CreateToken(LE,"<=");
                addToken(t);
            }

            else
            {
                Token *t = CreateToken(LT,"<");
                addToken(t);
                ungetc(next,fp);
            }
        }

        else if(ch == '>')
        {
            int next = fgetc(fp);
            if(next == '=')
            {
                Token *t = CreateToken(GE,">=");
                addToken(t);
            }

            else
            {
                Token *t = CreateToken(GT,">");
                addToken(t);
                ungetc(next,fp);
            }
        }

        else if(ch == '!')
        {
            
            int next = fgetc(fp);
            if(next == '=')
            {
                Token* t = CreateToken(NE,"!=");
                addToken(t);
            }
            else{
                Token* t = CreateToken(NOT, "!");
                addToken(t);
                ungetc(next,fp);
            }
        }
        else if (ch == '&') {
            char next = fgetc(fp);
            if (next == '&') {
                Token* t = CreateToken(AND, "&&");
                addToken(t);
            } else {
                printf("Lexical Error: unexpected '&'\n");
                exit(1);
            }
        }

        else if (ch == '|') {
            char next = fgetc(fp);
            if (next == '|') {
                Token* t = CreateToken(OR, "||");
                addToken(t);
            } else {
                printf("Lexical Error: unexpected '|'\n");
                exit(1);
            }
        }

        else if(isdigit(ch) || ch == '.')
        {
            char buffer[50];
            int c=0;
            int j=0;
            do{
                if(ch == '.'){
                    c+=1;
                }
                buffer[j++] = ch;
                ch = fgetc(fp);
            }while(isdigit(ch) || (ch=='.' && c == 0));

            buffer[j]='\0';
            if(ch != EOF)
            ungetc(ch,fp);
            Token *t = CreateToken(NUM, buffer);
            addToken(t);
        }

        else if(isalpha(ch) || ch == '_')
        {
            char buffer[32];
            int j=0;
            do
            {
                buffer[j++] = ch;
                ch = fgetc(fp);
            }while(isalnum(ch) || (ch == '_'));
            
            buffer[j] = '\0';
            ungetc(ch,fp);
            //check here to add as a keyword
            if(strcmp(buffer,"int") == 0){
                Token *t = CreateToken(INT , buffer);
                addToken(t);
                continue;
                }else if(strcmp(buffer, "float") == 0) {
                Token *t=CreateToken(FLOAT, buffer);
                addToken(t);
                continue;
            }else if(strcmp(buffer,"double")==0){
                Token *t=CreateToken(FLOAT, buffer);
                addToken(t);
                continue;
            }
            else if(strcmp(buffer,"start") == 0){
                Token *t = CreateToken(START , buffer);
                addToken(t);
                continue;
            }
            else if(strcmp(buffer,"end") == 0){
                Token *t = CreateToken(END , buffer);
                addToken(t);
                continue;   
            }
            else if(strcmp(buffer,"if") == 0){
                Token *t = CreateToken(IF , buffer);
                addToken(t);
                continue;   
            }
            else if(strcmp(buffer,"else") == 0){
                Token *t = CreateToken(ELSE , buffer);
                addToken(t);
                continue;   
            }
            else if(strcmp(buffer,"while") == 0){
                Token *t = CreateToken(WHILE , buffer);
                addToken(t);
                continue;   
            }
            else if(strcmp(buffer,"elif") == 0){
                Token *t = CreateToken(ELIF , buffer);
                addToken(t);
                continue;   
            }
            else if (strcmp(buffer,"print") == 0){
                Token *t = CreateToken(PRINT , buffer);
                addToken(t);
                continue;
            }
            else if (strcmp(buffer,"input") == 0)
            {
                Token *t = CreateToken(INPUT ,buffer);
                addToken(t);
                continue; 
            }
            Token *t = CreateToken(ID , buffer);
            addToken(t);
        }
    }

    Token *t = CreateToken(EOI , "EOF");
    addToken(t);

}
