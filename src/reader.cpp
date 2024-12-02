#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "reader.h"
#include "diff_tree.h"
#include "operations.h"

Node *GetExpr(Expression *expr, Tree *dest_tree)
{
    dest_tree->root_ptr = GetSum(expr, dest_tree);

    SkipSpaces(expr);
    if (expr->data[expr->ip] != END_OF_EXPR)
    {
        fprintf(stderr, "expr->data + expr->ip = '%s'\n", expr->data + expr->ip);
        SYNTAX_ERROR(expr, "expr->data[expr->ip] != END_OF_EXPR");
    }

    return dest_tree->root_ptr;
}

Node *GetSum(Expression *expr, Tree *dest_tree)
{
    Node *res_node = GetMul(expr, dest_tree);

    if (expr->data[expr->ip] == SEPARATOR)
        return res_node;

    char func_symbol[MAX_FUNCNAME_LEN] = {};
    int  shift = 0;

    SkipSpaces(expr);
    sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_symbol, &shift);

    const Operation *cur_op = GetOperationBySymbol(func_symbol);

    if (cur_op == NULL)
        return res_node;

    bool is_add = (cur_op->num == ADD);
    bool is_sub = (cur_op->num == SUB);

    while ((is_add || is_sub) && shift != 0)
    {
        expr->ip += shift;

        Node *arg_2 = GetMul(expr, dest_tree);

        res_node = NewNode(dest_tree, OP, cur_op->num, res_node, arg_2);

        shift = 0;
        SkipSpaces(expr);
        sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_symbol, &shift);

        cur_op = GetOperationBySymbol(func_symbol);

        if (cur_op == NULL)
        {
            fprintf(stderr, "operation: %s\n", func_symbol);
            SYNTAX_ERROR(expr, "unknown operation");
        }


        is_add = (cur_op->num == ADD);
        is_sub = (cur_op->num == SUB);
    }

    return res_node;
}

Node *GetMul(Expression *expr, Tree *dest_tree)
{
    Node *res_node = GetPow(expr, dest_tree);

    char func_symbol[MAX_FUNCNAME_LEN] = {};
    int  shift = 0;

    SkipSpaces(expr);
    sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_symbol, &shift);

    const Operation *cur_op = GetOperationBySymbol(func_symbol);

    if (cur_op == NULL)
        return res_node;

    bool is_mul = (cur_op->num == MUL);
    bool is_div = (cur_op->num == DIV);

    while ((is_mul || is_div) && shift != 0)
    {
        expr->ip += shift;

        Node *arg_2 = GetPow(expr, dest_tree);

        res_node = NewNode(dest_tree, OP, cur_op->num, res_node, arg_2);

        shift = 0;
        SkipSpaces(expr);
        sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_symbol, &shift);

        cur_op = GetOperationBySymbol(func_symbol);

        if (cur_op == NULL)
        {
            fprintf(stderr, "operation: '%s'\n", func_symbol);
            SYNTAX_ERROR(expr, "unknown operation");
        }

        is_mul = (cur_op->num == MUL);
        is_div = (cur_op->num == DIV);
    }

    return res_node;
}

Node *GetPow(Expression *expr, Tree *dest_tree)
{
    Node *got_node = GetFunc(expr, dest_tree);

    char func_symbol[MAX_FUNCNAME_LEN] = {};
    int  shift = 0;

    SkipSpaces(expr);
    sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_symbol, &shift);

    const Operation *cur_op = GetOperationBySymbol(func_symbol);

    if (cur_op == NULL)
        return got_node;

    if (cur_op->num == DEG)
    {
        expr->ip++;

        Node *degree = GetPow(expr, dest_tree);
        return NewNode(dest_tree, OP, DEG, got_node, degree);
    }

    else
        return got_node;
}

Node *GetFunc(Expression *expr, Tree *dest_tree)
{
    char func_name[MAX_FUNCNAME_LEN] = {};
    int  shift = 0;

    SkipSpaces(expr);
    sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_name, &shift);

    const Operation *op = GetOperationBySymbol(func_name);

    if (op != NULL)
    {
        expr->ip += shift;

        if (op->life_form == INFIX)
            SYNTAX_ERROR(expr, "op->life_form == INFIX");

        if (op->type == UNARY)
        {
            Node *arg = GetExprInBrackets(expr, dest_tree);
            return NewNode(dest_tree, OP, op->num, arg, arg);
        }

        else
        {
            if (expr->data[expr->ip++] != BRACKET_OPEN)
                SYNTAX_ERROR(expr, "no open bracket in arg of binary func");

            Node *arg_1 = GetExprInBrackets(expr, dest_tree);

            if (expr->data[expr->ip++] != SEPARATOR)
                SYNTAX_ERROR(expr, "no separator in arg of binary func");

            Node *arg_2 = GetExprInBrackets(expr, dest_tree);

            if (expr->data[expr->ip++] != BRACKET_CLOSE)
                SYNTAX_ERROR(expr, "no close bracket in arg of binary func");

            return NewNode(dest_tree, OP, op->num, arg_1, arg_2);
        }        
    }

    else 
        return GetExprInBrackets(expr, dest_tree);
}

Node *GetExprInBrackets(Expression *expr, Tree *dest_tree)
{
    if (expr->data[expr->ip] == BRACKET_OPEN)
    {
        expr->ip++;

        Node *res_node = GetSum(expr, dest_tree);

        if (expr->data[expr->ip] != BRACKET_CLOSE)
            SYNTAX_ERROR(expr, "expr->data[expr->ip] != BRACKET_CLOSE");

        expr->ip++;
        return res_node;
    }

    else 
        return GetNumber(expr, dest_tree);

}

Node *GetNumber(Expression *expr, Tree *dest_tree)
{
    TreeElem_t val = 0;

    SkipSpaces(expr);

    if ('a' <= expr->data[expr->ip] && expr->data[expr->ip] <= 'z')
    {
        val = expr->data[expr->ip];
        expr->ip++;

        return NewNode(dest_tree, VAR, val, NULL, NULL);
    }

    else if ('0' <= expr->data[expr->ip] && expr->data[expr->ip] <= '9')
    {
        while ('0' <= expr->data[expr->ip] && expr->data[expr->ip] <= '9')
        {
            val = val * 10 + (expr->data[expr->ip] - '0');
            expr->ip++;
        }

        return NewNode(dest_tree, NUM, val, NULL, NULL);
    }
    
    else
        SYNTAX_ERROR(expr, "incorrect syntax");

    return NewNode(dest_tree, NUM, val, NULL, NULL);
}

void SyntaxError(Expression *expr, const char *error, const char *file, int line, const char *func)
{
    fprintf(stderr, "SyntaxError called in %s:%d |%s()|, ip = %lld\nerror: '%s'\n", file, line, func, expr->ip, error);
    abort();
}

char *SkipSpaces(Expression *expr)
{
    while (isspace(expr->data[expr->ip]))
        expr->ip++;

    return expr->data + expr->ip;
}