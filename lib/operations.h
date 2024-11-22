#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdio.h>
#include "diff_tree.h"


enum Operation_enum
{
    ADD,
    SUB,
    MUL,
    DIV,
    DEG
};

TreeElem_t Add(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Sub(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Mul(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Div(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Deg(TreeElem_t arg1, TreeElem_t arg2);

enum FuncEntryForm
{
    IS_PREFIX,
    IS_INFIX
};

struct Operation
{
    const int   num;
    const char  symbol;
    const char *tex_code;
    TreeElem_t (*op_func) (TreeElem_t arg1, TreeElem_t arg2);
    const FuncEntryForm form;
};

const int OPERATIONS_NUM = 5;

struct Operations
{
    const size_t size;

    const Operation data[OPERATIONS_NUM];
};

const Operations OperationsArr = 
{
    .size = OPERATIONS_NUM,
    
    .data = {
        { .num = ADD, .symbol = '+', .tex_code = "+",      .op_func = Add, IS_INFIX  },
        { .num = SUB, .symbol = '-', .tex_code = "-",      .op_func = Sub, IS_INFIX  },
        { .num = MUL, .symbol = '*', .tex_code = "\\cdot", .op_func = Mul, IS_INFIX  },
        { .num = DIV, .symbol = '/', .tex_code = "\\frac", .op_func = Div, IS_PREFIX },
        { .num = DEG, .symbol = '^', .tex_code = "^",      .op_func = Deg, IS_INFIX  }
    }
};

#endif