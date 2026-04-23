#include "include/lexer.h"
#include "include/parser.h"
#include "include/AST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int current = 0;
Token* current_token;

static void syntax_error(const char* msg)
{
    if (current_token != NULL)
        printf("Syntax Error near token '%s': %s\n", current_token->lexeme, msg);
    else
        printf("Syntax Error: %s\n", msg);
    exit(1);
}

static void expect_token(int expected_type, const char* msg)
{
    if (current_token != NULL && current_token->type == expected_type) {
        advance();
        return;
    }
    syntax_error(msg);
}

Node* parse()
{
    current = 0;
    current_token = tokens[current];
    return parse_program();
}

void advance()
{
    if (current < tokencount)
        current++;

    if (current < tokencount)
        current_token = tokens[current];
    else
        current_token = NULL;
}

int match(int expected_type)
{
    if (current_token != NULL && current_token->type == expected_type) {
        advance();
        return 1;
    }

    syntax_error("unexpected token");
    return 0;
}

/* ---------------- PARAM ARRAY DIMENSIONS ---------------- */
/* Supports function params like:
   a(int)[]
   a(int)[][]
   a(int)[][][]
*/
Node* parse_param_dimensions()
{
    Node* head = NULL;
    Node* tail = NULL;

    while (current_token->type == LB) {
        Node* dim;

        match(LB);

        /* function parameter arrays must be unsized: [] */
        if (current_token->type != RB)
            syntax_error("array parameter must use empty brackets '[]'");

        dim = CreateNode("[]", NODE_CONST);
        match(RB);

        if (head == NULL) {
            head = tail = dim;
        } else {
            tail->next = dim;
            tail = dim;
        }
    }

    return head;
}

Node* parse_param_list()
{
    Node* head = NULL;
    Node* tail = NULL;

    if (current_token->type == RP)
        return NULL;

    while (1) {
        char param_name[50];
        char* type_name = NULL;
        Node* paramNode;
        Node* dims = NULL;

        if (current_token->type != ID)
            syntax_error("expected parameter name");

        strcpy(param_name, current_token->lexeme);
        match(ID);

        expect_token(LP, "expected '(' after parameter name");

        if (current_token->type == INT) {
            type_name = "int";
            match(INT);
        }
        else if (current_token->type == FLOAT) {
            type_name = "float";
            match(FLOAT);
        }
        else if (current_token->type == CHAR) {
            type_name = "char";
            match(CHAR);
        }
        else {
            syntax_error("expected parameter type");
        }

        expect_token(RP, "expected ')' after parameter type");

        /* NEW: parse [] / [][] / [][][] after param type */
        dims = parse_param_dimensions();

        paramNode = CreateNode("param", NODE_PARAM);
        paramNode->left = CreateNode(param_name, NODE_ID);
        paramNode->left->left = CreateNode(type_name, NODE_TYPE);

        /* Attach ND-array info to the parameter node.
           Convention:
           paramNode->left           = param identifier
           paramNode->left->left     = base type
           paramNode->left->right    = dimension list if array param
        */
        paramNode->left->right = dims;

        if (head == NULL) {
            head = tail = paramNode;
        } else {
            tail->next = paramNode;
            tail = paramNode;
        }

        if (current_token->type == COMMA)
            match(COMMA);
        else
            break;
    }

    return head;
}

Node* parse_arg_list()
{
    Node* head = NULL;
    Node* tail = NULL;

    if (current_token->type == RP)
        return NULL;

    while (1) {
        Node* argNode = CreateNode("arg", NODE_ARG);
        argNode->left = parse_expr();

        if (head == NULL) {
            head = tail = argNode;
        } else {
            tail->next = argNode;
            tail = argNode;
        }

        if (current_token->type == COMMA)
            match(COMMA);
        else
            break;
    }

    return head;
}

Node* parse_function_def()
{
    char func_name[50];
    Node* funcNode;
    Node* params;
    Node* body;

    match(FUNCTION);

    if (current_token->type != ID)
        syntax_error("expected function name");

    strcpy(func_name, current_token->lexeme);
    match(ID);

    expect_token(LP, "expected '(' after function name");
    params = parse_param_list();
    expect_token(RP, "expected ')' after parameter list");

    expect_token(START, "expected 'start' before function body");
    body = parse_stmt_list();
    expect_token(END, "expected 'end' after function body");

    funcNode = CreateNode("function", NODE_FUNC_DEF);
    funcNode->left = CreateNode(func_name, NODE_ID);
    funcNode->left->left = params;
    funcNode->right = body;

    return funcNode;
}

Node* parse_function_call(char* name)
{
    Node* callNode;
    Node* args;

    expect_token(LP, "expected '(' in function call");
    args = parse_arg_list();
    expect_token(RP, "expected ')' after function arguments");

    callNode = CreateNode("call", NODE_FUNC_CALL);
    callNode->left = CreateNode(name, NODE_ID);
    callNode->right = args;

    return callNode;
}

Node* parse_return_stmt()
{
    Node* retNode;

    match(RETURN);

    retNode = CreateNode("return", NODE_RETURN);

    if (current_token->type == SEMI) {
        match(SEMI);
        retNode->left = NULL;
        return retNode;
    }

    retNode->left = parse_expr();
    expect_token(SEMI, "expected ';' after return");

    return retNode;
}

Node* parse_array_assignment(char* name)
{
    Node* access = CreateNode(name, NODE_ID);

    while (current_token->type == LB) {
        Node* index;
        match(LB);
        index = parse_expr();
        expect_token(RB, "expected ']' after array index");
        access = make_array_access_chain_node(access, index);
    }

    expect_token(ASSIGN, "expected '=' after array access");
    Node* expr = parse_expr();
    expect_token(SEMI, "expected ';' after array assignment");

    return make_array_assign_node(access, expr);
}

Node* parse_array_dimensions()
{
    Node* head = NULL;
    Node* tail = NULL;

    while (current_token->type == LB) {
        Node* dim;

        match(LB);

        if (current_token->type != NUM_INT)
            syntax_error("array dimension must be an integer constant");

        dim = CreateNode(current_token->lexeme, NODE_CONST);
        match(NUM_INT);
        expect_token(RB, "expected ']' after array dimension");

        if (head == NULL) {
            head = tail = dim;
        } else {
            tail->next = dim;
            tail = dim;
        }
    }

    return head;
}

Node* parse_declaration()
{
    char var_name[50];
    char* type_name = NULL;

    strcpy(var_name, current_token->lexeme);
    match(ID);
    expect_token(LP, "expected '(' after identifier in declaration");

    if (current_token->type == INT) {
        type_name = "int";
        match(INT);
    }
    else if (current_token->type == FLOAT) {
        type_name = "float";
        match(FLOAT);
    }
    else if (current_token->type == CHAR) {
        type_name = "char";
        match(CHAR);
    }
    else {
        syntax_error("expected 'int' or 'float' or 'char'");
    }

    expect_token(RP, "expected ')' after type");

    if (current_token->type == LB) {
        Node* dims = parse_array_dimensions();
        expect_token(SEMI, "expected ';' after array declaration");
        return make_array_decl_nd_node(var_name, type_name, dims);
    }

    if (current_token->type == SEMI) {
        Node* n;
        match(SEMI);
        n = CreateNode("Declaration", NODE_DECL);
        n->left = CreateNode(var_name, NODE_ID);
        n->left->left = CreateNode(type_name, NODE_TYPE);
        return n;
    }

    if (current_token->type == ASSIGN) {
        Node* expr;
        Node* n;

        match(ASSIGN);
        expr = parse_expr();
        expect_token(SEMI, "expected ';' after declaration assignment");

        n = CreateNode("Decl_Assign", NODE_DECL_ASSN);
        n->left = CreateNode("=", NODE_ASSIGN);
        n->left->left = CreateNode(var_name, NODE_ID);
        n->left->left->left = CreateNode(type_name, NODE_TYPE);
        n->left->right = expr;
        return n;
    }

    syntax_error("expected ';', '[' or '=' after declaration");
    return NULL;
}

Node* parse_fact()
{
    if (current_token->type == ID) {
        char name[50];
        Node* node;

        strcpy(name, current_token->lexeme);
        advance();

        if (current_token->type == LP) {
            return parse_function_call(name);
        }

        node = CreateNode(name, NODE_ID);

        while (current_token->type == LB) {
            Node* index;
            match(LB);
            index = parse_expr();
            expect_token(RB, "expected ']' after array index");
            node = make_array_access_chain_node(node, index);
        }

        return node;
    }

    if (current_token->type == NOT) {
        Node* operand;
        Node* node;

        advance();
        operand = parse_fact();
        node = CreateNode("!", NODE_OP);
        node->left = operand;
        node->right = NULL;
        return node;
    }

    if (current_token->type == NUM_INT ||
        current_token->type == NUM_FLOAT ||
        current_token->type == CHAR_CONST) {
        Node* node = CreateNode(current_token->lexeme, NODE_CONST);
        advance();
        return node;
    }

    if (current_token->type == LP) {
        Node* node;
        match(LP);
        node = parse_expr();
        expect_token(RP, "expected ')'");
        return node;
    }

    syntax_error("expected number, identifier, function call, or '('");
    return NULL;
}

Node* parse_term()
{
    Node* left = parse_fact();

    while (current_token->type == DIV ||
           current_token->type == MUL ||
           current_token->type == MOD) {
        char* op = strdup(current_token->lexeme);
        Node* right;
        Node* node;

        advance();
        right = parse_fact();

        node = CreateNode(op, NODE_OP);
        node->left = left;
        node->right = right;
        left = node;
    }

    return left;
}

Node* parse_additive()
{
    Node* left = parse_term();

    while (current_token->type == ADD || current_token->type == SUB) {
        char* op = strdup(current_token->lexeme);
        Node* right;
        Node* node;

        advance();
        right = parse_term();

        node = CreateNode(op, NODE_OP);
        node->left = left;
        node->right = right;
        left = node;
    }

    return left;
}

Node* parse_relation()
{
    Node* left = parse_additive();

    while (current_token->type == LT ||
           current_token->type == GT ||
           current_token->type == LE ||
           current_token->type == GE) {
        char* op = strdup(current_token->lexeme);
        Node* right;
        Node* node;

        advance();
        right = parse_additive();

        node = CreateNode(op, NODE_OP);
        node->left = left;
        node->right = right;
        left = node;
    }

    return left;
}

Node* parse_equal()
{
    Node* left = parse_relation();

    while (current_token->type == EQ || current_token->type == NE) {
        char* op = strdup(current_token->lexeme);
        Node* right;
        Node* node;

        advance();
        right = parse_relation();

        node = CreateNode(op, NODE_OP);
        node->left = left;
        node->right = right;
        left = node;
    }

    return left;
}

Node* parse_logical_and()
{
    Node* left = parse_equal();

    while (current_token->type == AND) {
        char* op = strdup(current_token->lexeme);
        Node* right;
        Node* node;

        advance();
        right = parse_equal();

        node = CreateNode(op, NODE_OP);
        node->left = left;
        node->right = right;
        left = node;
    }

    return left;
}

Node* parse_logical_or()
{
    Node* left = parse_logical_and();

    while (current_token->type == OR) {
        char* op = strdup(current_token->lexeme);
        Node* right;
        Node* node;

        advance();
        right = parse_logical_and();

        node = CreateNode(op, NODE_OP);
        node->left = left;
        node->right = right;
        left = node;
    }

    return left;
}

Node* parse_assign()
{
    if (current_token->type == ID) {
        if ((current + 1) < tokencount && tokens[current + 1]->type == ASSIGN) {
            char name[50];
            Node* node;
            Node* right;

            strcpy(name, current_token->lexeme);
            advance();
            match(ASSIGN);

            right = parse_logical_or();
            expect_token(SEMI, "expected ';' after assignment");

            node = CreateNode("=", NODE_ASSIGN);
            node->left = CreateNode(name, NODE_ID);
            node->right = right;
            return node;
        }

        if ((current + 1) < tokencount && tokens[current + 1]->type == LB) {
            char name[50];
            int temp;
            int bracket_count = 0;

            strcpy(name, current_token->lexeme);
            temp = current + 1;

            while (temp < tokencount) {
                if (tokens[temp]->type == LB) bracket_count++;
                else if (tokens[temp]->type == RB) {
                    bracket_count--;
                    if (bracket_count == 0) {
                        int next = temp + 1;
                        while (next < tokencount && tokens[next]->type == LB) {
                            temp = next;
                            bracket_count = 0;
                            while (temp < tokencount) {
                                if (tokens[temp]->type == LB) bracket_count++;
                                else if (tokens[temp]->type == RB) {
                                    bracket_count--;
                                    if (bracket_count == 0) break;
                                }
                                temp++;
                            }
                            next = temp + 1;
                        }
                        break;
                    }
                }
                temp++;
            }

            if ((temp + 1) < tokencount && tokens[temp + 1]->type == ASSIGN) {
                advance();
                return parse_array_assignment(name);
            }
        }
    }

    return parse_logical_or();
}

Node* parse_expr()
{
    return parse_assign();
}

Node* parse_print()
{
    Node* head = NULL;
    Node* tail = NULL;

    match(PRINT);
    expect_token(LP, "expected '(' after print");

    while (1) {
        Node* print_node = NULL;

        if (current_token->type == STRING) {
            print_node = CreateNode(current_token->lexeme, NODE_STRING);
            match(STRING);
        } else {
            print_node = parse_expr();
        }

        if (head == NULL) {
            head = tail = print_node;
        } else {
            tail->next = print_node;
            tail = print_node;
        }

        if (current_token->type == COMMA)
            match(COMMA);
        else
            break;
    }

    expect_token(RP, "expected ')' after print arguments");
    expect_token(SEMI, "expected ';' after print");

    {
        Node* node = CreateNode("print", NODE_PRINT);
        node->left = head;
        return node;
    }
}

Node* parse_input_stmt()
{
    Node* targetNode;
    Node* inNode;
    char name[100];

    match(INPUT);
    expect_token(LP, "expected '(' after input");

    if (current_token->type != ID)
        syntax_error("expected identifier inside input()");

    strcpy(name, current_token->lexeme);
    advance();

    if (current_token->type == LB) {
        Node* indexList = NULL;
        Node* indexTail = NULL;

        while (current_token->type == LB) {
            Node* idx;

            match(LB);
            idx = parse_expr();
            expect_token(RB, "expected ']' after array index");

            idx->next = NULL;
            if (indexList == NULL) {
                indexList = idx;
                indexTail = idx;
            } else {
                indexTail->next = idx;
                indexTail = idx;
            }
        }

        targetNode = make_array_access_node(name, indexList);
    } 
    else {
        targetNode = CreateNode(name, NODE_ID);
    }

    expect_token(RP, "expected ')' after input argument");
    expect_token(SEMI, "expected ';' after input");

    inNode = CreateNode("input", NODE_INPUT);
    inNode->left = targetNode;

    return inNode;
}

Node* parse_if()
{
    Node* condNode;
    Node* thenNode = NULL;
    Node* ifNode;
    Node* currentIf;

    match(IF);
    expect_token(LP, "expected '(' after if");
    condNode = parse_logical_or();
    expect_token(RP, "expected ')' after if condition");

    if (current_token->type == START) {
        match(START);
        thenNode = parse_stmt_list();
        expect_token(END, "expected 'end' after if block");
    } else {
        thenNode = parse_statement();
    }

    ifNode = CreateNode("if", NODE_IF);
    ifNode->left = condNode;
    ifNode->right = thenNode;
    currentIf = ifNode;

    while (current_token->type == ELIF) {
        Node* elifCond;
        Node* elifThen = NULL;
        Node* elifNode;
        Node* elseWrap;

        match(ELIF);
        expect_token(LP, "expected '(' after elif");
        elifCond = parse_logical_or();
        expect_token(RP, "expected ')' after elif condition");

        if (current_token->type == START) {
            match(START);
            elifThen = parse_stmt_list();
            expect_token(END, "expected 'end' after elif block");
        } else {
            elifThen = parse_statement();
        }

        elifNode = CreateNode("if", NODE_IF);
        elifNode->left = elifCond;
        elifNode->right = elifThen;

        elseWrap = CreateNode("else", NODE_ELSE);
        elseWrap->left = currentIf->right;
        elseWrap->right = elifNode;

        currentIf->right = elseWrap;
        currentIf = elifNode;
    }

    if (current_token->type == ELSE) {
        Node* elseNode = NULL;
        Node* elseWrap;

        match(ELSE);

        if (current_token->type == START) {
            match(START);
            elseNode = parse_stmt_list();
            expect_token(END, "expected 'end' after else block");
        } else {
            elseNode = parse_statement();
        }

        elseWrap = CreateNode("else", NODE_ELSE);
        elseWrap->left = currentIf->right;
        elseWrap->right = elseNode;

        currentIf->right = elseWrap;
    }

    return ifNode;
}

Node* parse_for()
{
    Node* init;
    Node* cond;
    Node* inc;
    Node* body = NULL;
    Node* temp;
    Node* whileNode;

    match(FOR);
    expect_token(LP, "expected '(' after for");

    if (current_token->type == ID &&
        (current + 1) < tokencount &&
        tokens[current + 1]->type == LP) {
        init = parse_declaration();
    } else {
        if (current_token->type == ID &&
            (current + 1) < tokencount &&
            tokens[current + 1]->type == ASSIGN) {
            char name[50];
            Node* right;
            strcpy(name, current_token->lexeme);
            advance();
            match(ASSIGN);
            right = parse_logical_or();

            init = CreateNode("=", NODE_ASSIGN);
            init->left = CreateNode(name, NODE_ID);
            init->right = right;
        }
        else {
            syntax_error("invalid for-loop initializer");
            return NULL;
        }
        expect_token(SEMI, "expected ';' after for init");
    }

    cond = parse_expr();
    expect_token(SEMI, "expected ';' after for condition");

    if (current_token->type == ID &&
        (current + 1) < tokencount &&
        tokens[current + 1]->type == ASSIGN) {
        char name[50];
        Node* right;
        strcpy(name, current_token->lexeme);
        advance();
        match(ASSIGN);
        right = parse_logical_or();

        inc = CreateNode("=", NODE_ASSIGN);
        inc->left = CreateNode(name, NODE_ID);
        inc->right = right;
    }
    else {
        syntax_error("invalid for-loop increment");
        return NULL;
    }

    expect_token(RP, "expected ')' after for");

    if (current_token->type == START) {
        match(START);
        body = parse_stmt_list();
        expect_token(END, "expected 'end' after for block");
    } else {
        body = parse_statement();
    }

    temp = body;
    if (temp == NULL) {
        body = inc;
    } else {
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = inc;
    }

    whileNode = CreateNode("while", NODE_WHILE);
    whileNode->left = cond;
    whileNode->right = body;

    init->next = whileNode;
    return init;
}

Node* parse_while()
{
    Node* condNode;
    Node* bodyNode = NULL;
    Node* whileNode;

    match(WHILE);
    expect_token(LP, "expected '(' after while");
    condNode = parse_logical_or();
    expect_token(RP, "expected ')' after while condition");

    if (current_token->type == START) {
        match(START);
        bodyNode = parse_stmt_list();
        expect_token(END, "expected 'end' after while block");
    } else {
        bodyNode = parse_statement();
    }

    whileNode = CreateNode("while", NODE_WHILE);
    whileNode->left = condNode;
    whileNode->right = bodyNode;

    return whileNode;
}

Node* parse_main_block()
{
    Node* body;
    Node* mainNode;

    match(MAIN);
    expect_token(START, "expected 'start' after main");

    body = parse_stmt_list();

    expect_token(END, "expected 'end' after main block");

    mainNode = CreateNode("main", NODE_MAIN);
    mainNode->left = body;
    mainNode->right = NULL;

    return mainNode;
}

Node* parse_statement()
{
    if (current_token->type == FUNCTION) {
        return parse_function_def();
    }
    else if (current_token->type == MAIN) {
        return parse_main_block();
    }
    else if (current_token->type == RETURN) {
        return parse_return_stmt();
    }
    else if (current_token->type == ID) {
        if ((current + 1) < tokencount && tokens[current + 1]->type == LP) {
            int look = current + 2;
            int is_decl = 0;

            while (look < tokencount && tokens[look]->type != RP) {
                if (tokens[look]->type == INT ||
                    tokens[look]->type == FLOAT ||
                    tokens[look]->type == CHAR) {
                    is_decl = 1;
                    break;
                }
                look++;
            }

            if (is_decl)
                return parse_declaration();
        }

        {
            Node* expr = parse_assign();

            if (expr != NULL && expr->type == NODE_FUNC_CALL)
                expect_token(SEMI, "expected ';' after function call");

            return expr;
        }
    }
    else if (current_token->type == IF) {
        return parse_if();
    }
    else if (current_token->type == WHILE) {
        return parse_while();
    }
    else if (current_token->type == FOR) {
        return parse_for();
    }
    else if (current_token->type == PRINT) {
        return parse_print();
    }
    else if (current_token->type == START) {
        Node* block;
        match(START);
        block = parse_stmt_list();
        expect_token(END, "expected 'end' after block");
        return block;
    }
    else if (current_token->type == SEMI) {
        advance();
        return NULL;
    }
    else if (current_token->type == INPUT) {
        return parse_input_stmt();
    }

    syntax_error("invalid statement");
    return NULL;
}

Node* parse_stmt_list()
{
    Node* head = NULL;
    Node* tail = NULL;

    while (current_token != NULL &&
           current_token->type != END &&
           current_token->type != EOI) {
        Node* stmtNode = parse_statement();

        if (stmtNode == NULL)
            continue;

        if (head == NULL) {
            head = stmtNode;
            tail = stmtNode;
        } else {
            tail->next = stmtNode;
        }

        while (tail->next != NULL)
            tail = tail->next;
    }

    return head;
}

Node* parse_program()
{
    Node* head = NULL;
    Node* tail = NULL;
    int main_found = 0;

    while (current_token != NULL && current_token->type != EOI) {
        Node* node = NULL;

        if (current_token->type == SEMI) {
            advance();
            continue;
        }

        if (current_token->type == FUNCTION) {
            node = parse_function_def();
        }
        else if (current_token->type == MAIN) {
            if (main_found)
                syntax_error("multiple main blocks are not allowed");

            node = parse_main_block();
            main_found = 1;
        }
        else {
            syntax_error("only function definitions and main block are allowed at program level");
        }

        node->next = NULL;

        if (head == NULL) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    if (!main_found)
        syntax_error("main block missing");

    return head;
}