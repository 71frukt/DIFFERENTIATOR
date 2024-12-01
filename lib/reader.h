#ifndef READER_H
#define READER_H

#include <stdio.h>

#include "diff_tree.h"

const int  EXPRESSION_LEN   = 400;
const int  MAX_FUNCNAME_LEN = 20;

const char END_OF_EXPR    = '$';
const char BRACKET_OPEN   = '(';
const char BRACKET_CLOSE  = ')';
const char SEPARATOR      = ',';

#define GET_FUNC_SPECIFIER  " %[a-zA-Z-+^*/]"

struct Expression
{
    char data[EXPRESSION_LEN];
    size_t ip;
};

TreeElem_t GetExpr           (Expression *expr);
TreeElem_t GetSum            (Expression *expr);
TreeElem_t GetMul            (Expression *expr);
TreeElem_t GetFunc           (Expression *expr);
TreeElem_t GetPow            (Expression *expr);
TreeElem_t GetExprInBrackets (Expression *expr);
TreeElem_t GetNumber         (Expression *expr);

void SyntaxError(Expression *expr, const char *error, const char *file, int line, const char *func);

#define SYNTAX_ERROR(expr, error)  SyntaxError(expr, error, __FILE__, __LINE__, __func__)

#endif