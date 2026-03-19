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
    strcpy(t->lexeme,lexeme);
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
}

int  keyword_checking( char buffer[])
{
    if (strcmp(buffer,"char") == 0 || strcmp(buffer,"int ") == 0 || strcmp(buffer,"while") == 0 || strcmp(buffer,"double") == 0 || strcmp(buffer,"float") == 0 || strcmp(buffer,"if") == 0 || strcmp(buffer,"else") == 0 || strcmp(buffer,"long") == 0 )
    {
        Token *t =CreateToken(KEY , buffer);
        addToken(t);
        return 1;
    }
    return 0;
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

        else if(ch==')')
        {
            Token *t =CreateToken(RP, ")");
            addToken(t);       
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
            else
            ungetc(next,fp);
        }

        else if(isdigit(ch) || ch == '.')
        {
            char buffer[50];
            int c=0;
            int j=0;
            do{
                if(ch == '.'){
                    c+=1;
                    if(!isdigit(getc(fp))){
                        
                        break;
                    }
                }
                buffer[j++] = ch;
                ch = fgetc(fp);
            }while(isdigit(ch) || (ch=='.' && c == 0));

            buffer[j]='\0';
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
            if(keyword_checking(buffer))
            continue;
            Token *t = CreateToken(ID , buffer);
            addToken(t);
        }
    }

    Token *t = CreateToken(EOI , "EOF");
    addToken(t);

}
