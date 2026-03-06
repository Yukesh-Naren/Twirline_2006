#include<stdio.h>
#include<stdlib.h>
#include "lexer.h"
#include<string.h>
#include <ctype.h>

Token tokens[100];
int tokencount = 0;

void tokenize(char* ch)
{
    int i = 0;
    while(ch[i]!=0)
    {
        Token t;
        t.lexeme[0]='\0';
        if(ch[i]=='+'){
            strcpy(t.lexeme,"+");
            t.type = ADD;
            tokens[tokencount++] = t ;
            i++;
        }

        else if(ch[i]=='-'){
            strcpy(t.lexeme,"-");
            t.type = SUB;
            tokens[tokencount++] = t ;
            i++;
        }

        else if(ch[i]=='*'){
            strcpy(t.lexeme,"*");
            t.type= MUL;
            tokens[tokencount++] = t;
            i++;
        }

        else if(ch[i]=='/'){
            strcpy(t.lexeme,"/");
            t.type = DIV ; 
            tokens[tokencount++] = t; 
            i++;
        }

        else if(ch[i]=='%'){
            strcpy(t.lexeme,"%");
            t.type = MOD ; 
            tokens[tokencount++] = t; 
            i++;
        }
        else if(ch[i]=='('){
            strcpy(t.lexeme,"(");
            t.type = LP;
            tokens[tokencount++]=t;
            i++;
        }

        else if(ch[i]==')'){
            strcpy(t.lexeme,")");
            t.type = RP;
            tokens[tokencount++]=t;
            i++;
        }

        else if(ch[i]=='='){
            if(ch[i+1]=='=')
            {
                strcpy(t.lexeme,"==");
                t.type = EQ;
                tokens[tokencount++] = t;
                i+=2;
            }
            else{
                strcpy(t.lexeme,"=");
                t.type = ASSIGN;
                tokens[tokencount++]=t;
                i++;
            }
        }

        else if(ch[i] == '<')
        {
            if(ch[i+1] == '='){
                strcpy(t.lexeme,"<=");
                t.type = LE;
                tokens[tokencount++] = t ;
                i+=2;
            }
            else{
                strcpy(t.lexeme,"<");
                t.type = LT;
                tokens[tokencount++]=t;
                i++;
            }
        }

        else if(ch[i] == '>')
        {
            if(ch[i+1] == '='){
                strcpy(t.lexeme,">=");
                t.type = GE;
                tokens[tokencount++] = t ;
                i+=2;
            }
            else{
                strcpy(t.lexeme,">");
                t.type = GT;
                tokens[tokencount++] = t;
                i++;
            }
        }

        else if(ch[i] == '!')
        {
            if(ch[i+1] == '='){
                strcpy(t.lexeme,"!=");
                t.type = NE;
                tokens[tokencount++] = t;
                i+=2;
            }

            else{
                i++;
            }
        }
        else if(isdigit(ch[i])){
            int c=0 ; // This c is used to check whether the decimal point has occurred before or not.
            int j=0;
            while(isdigit(ch[i]) || (ch[i] == '.' && c == 0)){
                if(ch[i]=='.'){
                    c += 1 ;
                }
                t.lexeme[j++] = ch [i++];
            }
            t.lexeme[j]='\0';
            t.type = NUM;
            tokens[tokencount++]=t;
        }

        else if (isalpha(ch[i]) || ch[i]=='_')
        {
            int j=0;
            while(isalnum(ch[i]) || (ch[i] == '_'))
            {
                t.lexeme[j++] = ch [i++];
            }
            t.lexeme[j]='\0';
            t.type = ID;
            tokens[tokencount++] = t;            
        }

        else {
            i++;
        }
    }

    Token t;
    strcpy(t.lexeme,"EOF");
    t.type=EOI;
    tokens[tokencount++] = t; 
}