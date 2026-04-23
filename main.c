#include<stdio.h>
#include<stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "AST.h"
#include "semantics.h"
#include "TAC.h"
#include "codegen.h"
#include <string.h>

int has_frx_extension(const char *filename) {
    int len = strlen(filename);

    if (len < 4)
        return 0;

    return strcmp(filename + len - 4, ".frx") == 0;
}


int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("Usage: ferrox <source_file>\n");
        return 1;
    }
    FILE *fp;

    char *filename = argv[1];

    if (!has_frx_extension(filename)) {
        printf("Error: Only .frx files are allowed\n");
        return 0;
    }

    fp=fopen(filename,"r");

    if(fp == NULL)
    {
        printf("File Not Found\n");
        return 0;
    }

    initTokens();
    GetNextToken(fp);
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
    // print_TAC();
    // printf("\nThree Address Code is Successful\n\n");
    
    generate_riscv_code();
    // printf("RISC- V Code Generation Successful\n");
    
    FILE *f = fopen("a.out", "w");

    fprintf(f,
    "#!/bin/bash\n"
    "java -jar rars.jar output.s\n");

    fclose(f);

    system("chmod +x a.out");

    freeAST(root);
}
