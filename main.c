#include<stdio.h>
#include"lexer.h"

int main()
{
    char input[200];
    // printf("Enter Expression: ");
    // fgets(input,sizeof(input),stdin);

    FILE *fp;
    char ch;

    char filename[100];

    printf("Enter the File Name: ");
    scanf("%s", filename);
    fp=fopen(filename,"r");

    if(fp == NULL)
    {
        printf("File Not Found\n");
        return 0;
    }

    initTokens();

    GetNextToken(fp);

    for(int i =0 ;i < tokencount; i++)
    printf("%s -> %d \n ", tokens[i]->lexeme , tokens[i]->type);

    fclose(fp);
    return 0;
}