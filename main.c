#include<stdio.h>
#include "lexer.h"
#include "parser.h"
#include "AST.h"
#include "semantics.h"
#include "TAC.h"
#include "codegen.h"
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
    // for(int i =0 ;i < tokencount; i++)
    // printf("%s -> %d \n ", tokens[i]->lexeme , tokens[i]->type);
    fclose(fp);

    Node* root = parse();

    // if(root!=NULL){
    //     printf("\nAST Tree : \n");
    //     printProgram(root);        
    // }
    //  printf("Parsing Successful\n");

    // printf("\nRunning Semantic Analysis...\n");

    check_semantics(root);
    // print_symbol_table();
    // printf("\nSemantic Analysis Completed Successfully\n");

    // printf("Starting TAC generation...\n");
    Node* current = root;
    
    generate_TAC(root);
    print_TAC();
    printf("\nThree Address Code is Successful");
    
    generate_riscv_code();
    printf("RISC- V Code Generation Successful\n");
    
    // freeTAC();
    freeAST(root);
}