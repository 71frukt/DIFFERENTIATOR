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

#define GET_FUNC_SPECIFIER  "%[a-zA-Z-+^*/]"

struct Expression
{
    char data[EXPRESSION_LEN];
    size_t ip;
};

Node *GetExpr           (Expression *expr, Tree *dest_tree);
Node *GetSum            (Expression *expr, Tree *dest_tree);
Node *GetMul            (Expression *expr, Tree *dest_tree);
Node *GetFunc           (Expression *expr, Tree *dest_tree);
Node *GetPow            (Expression *expr, Tree *dest_tree);
Node *GetExprInBrackets (Expression *expr, Tree *dest_tree);
Node *GetNumber         (Expression *expr, Tree *dest_tree);

void  SyntaxError (Expression *expr, const char *error, const char *file, int line, const char *func);
char *SkipSpaces  (Expression *expr);

#define SYNTAX_ERROR(expr, error)  SyntaxError(expr, error, __FILE__, __LINE__, __func__)

#endif