#include<stdio.h>
#include"lexer.h"

int main()
{
    char input[200];
    printf("Enter Expression: ");
    fgets(input,sizeof(input),stdin);

    tokenize(input);

    printf("\nTokens:\n");

    for(int i=0; i < tokencount; i++)
    {
        printf("Lexeme: %-10s Type: %d\n", tokens[i].lexeme, tokens[i].type);
    }
    return 0;
}