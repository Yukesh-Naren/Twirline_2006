#include<stdio.h>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/AST.h"
int main()
{
    FILE *fp;

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

    Node* root = parse();

    printf("\nAST Tree : \n");
    printProgram(root);
    return 0;
}