#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "reader.h"
#include "diff_tree.h"
#include "operations.h"

TreeElem_t GetExpr(Expression *expr)
{
    TreeElem_t val = GetSum(expr);

    if (expr->data[expr->ip] != END_OF_EXPR)
        SYNTAX_ERROR(expr, "expr->data[expr->ip] != END_OF_EXPR");

    return val;
}

TreeElem_t GetSum(Expression *expr)
{
    TreeElem_t val = GetMul(expr);

    if (expr->data[expr->ip] == SEPARATOR)
        return val;

    const char *plus_symbol  = GetOperationByNum(ADD)->symbol;
    const char *minus_symbol = GetOperationByNum(SUB)->symbol;

    char func_symbol[MAX_FUNCNAME_LEN] = {};
    int  shift = 0;

    sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_symbol, &shift);

    bool is_plus  = (strcmp(func_symbol, plus_symbol)  == 0);
    bool is_minus = (strcmp(func_symbol, minus_symbol) == 0);

    while ((is_plus || is_minus) && shift != 0)
    {
        expr->ip += shift;

        TreeElem_t val_2 = GetMul(expr);

        if (is_plus)
            val += val_2;

        else 
            val -= val_2;
            
        shift = 0;
        sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_symbol, &shift);

        is_plus  = (strcmp(func_symbol, plus_symbol)  == 0);
        is_minus = (strcmp(func_symbol, minus_symbol) == 0);
    }

    return val;
}

TreeElem_t GetMul(Expression *expr)
{
    TreeElem_t val = GetFunc(expr);

    const char *mul_symbol = GetOperationByNum(MUL)->symbol;
    const char *div_symbol = GetOperationByNum(DIV)->symbol;

    char func_symbol[MAX_FUNCNAME_LEN] = {};
    int  shift = 0;

    sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_symbol, &shift);

    bool is_mul = (strcmp(func_symbol, mul_symbol) == 0);
    bool is_div = (strcmp(func_symbol, div_symbol) == 0);

    while ((is_mul || is_div) && shift != 0)
    {
        expr->ip += shift;

        TreeElem_t val_2 = GetFunc(expr);

        if (is_mul)
            val *= val_2;

        else 
            val /= val_2;
            
        shift = 0;
        sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_symbol, &shift);

        is_mul = (strcmp(func_symbol, mul_symbol) == 0);
        is_div = (strcmp(func_symbol, div_symbol) == 0);
    }

    return val;
}

TreeElem_t GetFunc(Expression *expr)
{
    char func_name[MAX_FUNCNAME_LEN] = {};
    int  shift = 0;

    sscanf(expr->data + expr->ip, GET_FUNC_SPECIFIER "%n", func_name, &shift);
fprintf(stderr, "func_name = '%s'\n", func_name);
    const Operation *op = GetOperationBySymbol(func_name);

    if (op != NULL)
    {
        expr->ip += shift;

        TreeElem_t arg = 0;

        if (op->life_form == INFIX)
            SYNTAX_ERROR(expr, "op->life_form == INFIX");

        if (op->type == UNARY)
        {
            arg = GetPow(expr);
            TreeElem_t res = op->op_func(arg, arg);         // впоследствии логика дерева будет выстроена так, что условно для унарных функций два равных аргумента (одно и то же поддерево и правый сосед и левый одновременно)
            
            return res;
        }

        else
        {
            if (expr->data[expr->ip++] != BRACKET_OPEN)
                SYNTAX_ERROR(expr, "no open bracket in arg of binary func");

            TreeElem_t arg_1 = GetPow(expr);

            if (expr->data[expr->ip++] != SEPARATOR)
                SYNTAX_ERROR(expr, "no separator in arg of binary func");

            TreeElem_t arg_2 = GetPow(expr);

            if (expr->data[expr->ip++] != BRACKET_CLOSE)
                SYNTAX_ERROR(expr, "no close bracket in arg of binary func");

            TreeElem_t res = op->op_func(arg_1, arg_2);
            
            return res;
        }        
    }

    else 
        return GetPow(expr);
}


TreeElem_t GetPow(Expression *expr)
{
    TreeElem_t val = GetExprInBrackets(expr);

    const char pow_symbol = *GetOperationByNum(DEG)->symbol;

    if (expr->data[expr->ip] == pow_symbol)
    {
        expr->ip++;

        TreeElem_t degree = GetPow(expr);
        val = (TreeElem_t) pow(val, degree);
    }

    fprintf(stderr, "ip in pow = %lld\n", expr->ip);

    return val;
}

TreeElem_t GetExprInBrackets(Expression *expr)
{
    if (expr->data[expr->ip] == BRACKET_OPEN)
    {
        expr->ip++;

        TreeElem_t val = GetSum(expr);

        if (expr->data[expr->ip] != BRACKET_CLOSE)
            SYNTAX_ERROR(expr, "expr->data[expr->ip] != BRACKET_CLOSE");

        expr->ip++;
        return val;
    }

    else 
        return GetNumber(expr);

}

TreeElem_t GetNumber(Expression *expr)
{
    TreeElem_t val = 0;

    size_t old_ip = expr->ip;

    while ('0' <= expr->data[expr->ip] && expr->data[expr->ip] <= '9')
    {
        val = val * 10 + (expr->data[expr->ip] - '0');
        expr->ip++;
    }

    if (expr->ip == old_ip)
        SYNTAX_ERROR(expr, "expr->ip == old_ip");

    return val;
}

void SyntaxError(Expression *expr, const char *error, const char *file, int line, const char *func)
{
    fprintf(stderr, "SyntaxError called in %s:%d |%s()|, ip = %lld\nerror: '%s'\n", file, line, func, expr->ip, error);
    abort();
}