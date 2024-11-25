#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdio.h>
#include "diff_tree.h"


TreeElem_t Add(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Sub(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Mul(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Div(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Deg(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Ln (TreeElem_t arg1, TreeElem_t arg2);

TreeElem_t Sin(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Cos(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Tan(TreeElem_t arg1, TreeElem_t arg2);


// diff
Node *DiffAdd(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffMul(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffSub(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffDiv(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffLn (Tree *expr_tree, Node *expr_node, Tree *solv_tree);

Node *DiffSin(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffCos(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffTan(Tree *expr_tree, Node *expr_node, Tree *solv_tree);

enum FuncEntryForm
{
    IS_PREFIX,
    IS_INFIX
};

enum FuncType
{
    UNARY,
    BINARY
};

struct Operation
{
    const int    num;
    const char  *symbol;
    const char  *tex_code;
    const FuncType type;                                                // UNARY / BINARY
    const FuncEntryForm form;                                           // IS_PREFIX / IS_INFIX

    TreeElem_t  (*op_func)   (TreeElem_t arg1, TreeElem_t arg2);
    Node*       (*diff_func) (Tree *expr_tree, Node *expr_node, Tree *solv_tree);
};


enum Operation_enum
{
    ADD,
    SUB,
    MUL,
    DIV,
    DEG,
    LN,

    SIN,
    COS,
    TAN,

    DIF
};

const int OPERATIONS_NUM = 10;

const Operation Operations[OPERATIONS_NUM] = 
{
    { .num = ADD, .symbol = "+",   .tex_code = "+",      BINARY, IS_INFIX,  .op_func = Add, .diff_func = DiffAdd },
    { .num = SUB, .symbol = "-",   .tex_code = "-",      BINARY, IS_INFIX,  .op_func = Sub, .diff_func = DiffSub },
    { .num = MUL, .symbol = "*",   .tex_code = "\\cdot", BINARY, IS_INFIX,  .op_func = Mul, .diff_func = DiffMul },
    { .num = DIV, .symbol = "/",   .tex_code = "\\frac", BINARY, IS_PREFIX, .op_func = Div, .diff_func = DiffDiv },
    { .num = DEG, .symbol = "^",   .tex_code = "^",      BINARY, IS_INFIX,  .op_func = Deg/*, .diff_func = DegDiff */},
    { .num = LN,  .symbol = "ln",  .tex_code = "\\ln",   UNARY,  IS_PREFIX, .op_func = Ln , .diff_func = DiffLn  },
 
    { .num = SIN, .symbol = "sin", .tex_code = "\\sin",  UNARY,  IS_PREFIX, .op_func = Sin, .diff_func = DiffSin },
    { .num = COS, .symbol = "cos", .tex_code = "\\cos",  UNARY,  IS_PREFIX, .op_func = Cos, .diff_func = DiffCos },
    { .num = TAN, .symbol = "tan", .tex_code = "\\tan",  UNARY,  IS_PREFIX, .op_func = Tan, .diff_func = DiffTan },

    { .num = DIF, .symbol = "d",   .tex_code = "\\dd",    UNARY, IS_PREFIX, .op_func = NULL }
};

const Operation *GetOperationByNum    (int num);
const Operation *GetOperationBySymbol (char *sym);

#endif