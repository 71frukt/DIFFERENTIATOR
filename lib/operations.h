#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdio.h>
#include "diff_tree.h"


TreeElem_t Add(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Sub(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Mul(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Div(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Deg(TreeElem_t arg1, TreeElem_t arg2);

TreeElem_t Tan(TreeElem_t arg1, TreeElem_t arg2);

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
    const int   num;
    const char *symbol;
    const char *tex_code;
    TreeElem_t (*op_func) (TreeElem_t arg1, TreeElem_t arg2);
    const FuncType type;                                                // UNARY / BINARY
    const FuncEntryForm form;                                           // IS_PREFIX / IS_INFIX
};


enum Operation_enum
{
    ADD,
    SUB,
    MUL,
    DIV,
    DEG,
    TAN
};

const int OPERATIONS_NUM = 6;

struct OperationsArr
{
    const size_t size;

    const Operation data[OPERATIONS_NUM];
};

const OperationsArr Operations = 
{
    .size = OPERATIONS_NUM,
    
    .data = {
        { .num = ADD, .symbol = "+"  , .tex_code = "+",      .op_func = Add, BINARY, IS_INFIX  },
        { .num = SUB, .symbol = "-"  , .tex_code = "-",      .op_func = Sub, BINARY, IS_INFIX  },
        { .num = MUL, .symbol = "*"  , .tex_code = "\\cdot", .op_func = Mul, BINARY, IS_INFIX  },
        { .num = DIV, .symbol = "/"  , .tex_code = "\\frac", .op_func = Div, BINARY, IS_PREFIX },
        { .num = DEG, .symbol = "^"  , .tex_code = "^",      .op_func = Deg, BINARY, IS_INFIX  },
        { .num = TAN, .symbol = "tan", .tex_code = "\\tan",  .op_func = Tan,  UNARY, IS_PREFIX  }
    }
};

const Operation *GetOperationByNum    (int num);
const Operation *GetOperationBySymbol (char *sym);

#endif