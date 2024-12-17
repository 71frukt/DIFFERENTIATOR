#ifndef READER_H
#define READER_H

#include <stdio.h>

#include "diff_tree.h"

const int  EXPRESSION_LEN   = 400;
const int  MAX_FUNCNAME_LEN = 20;

#define    END_OF_TRANSLATION  "$"

#define GET_FUNC_SPECIFIER  "%[a-zA-Z-+^*/]"

struct Expression
{
    char data[EXPRESSION_LEN];
    size_t ip;
};

void GetTreeData (Tree *tree, FILE *source);
void MakeTokens  (Tree *tree, char *str);

Node *GetExpr           (Tree *dest_tree);
Node *GetSum            (Tree *dest_tree, size_t *ip);
Node *GetMul            (Tree *dest_tree, size_t *ip);
Node *GetFunc           (Tree *dest_tree, size_t *ip);
Node *GetPow            (Tree *dest_tree, size_t *ip);
Node *GetExprInBrackets (Tree *dest_tree, size_t *ip);
Node *GetNumber         (Tree *dest_tree, size_t *ip);

void  SyntaxError (size_t ip, const char *error, const char *file, int line, const char *func);
char *SkipSpaces  (Expression *expr);
bool  IsBracket   (Node *node);

#define SYNTAX_ERROR(ip, error)  SyntaxError(ip, error, __FILE__, __LINE__, __func__)

#endif