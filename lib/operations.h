#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdio.h>
#include "diff_tree.h"
#include "simplifier.h"


TreeElem_t Add(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Sub(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Mul(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Div(TreeElem_t arg1, TreeElem_t arg2);

TreeElem_t Deg(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Ln (TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Log(TreeElem_t arg1, TreeElem_t arg2);

TreeElem_t Sin(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Cos(TreeElem_t arg1, TreeElem_t arg2);
TreeElem_t Tan(TreeElem_t arg1, TreeElem_t arg2);


// diff
Node *DiffAdd(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffMul(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffSub(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffDiv(Tree *expr_tree, Node *expr_node, Tree *solv_tree);

Node *DiffDeg(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffLn (Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffLog(Tree *expr_tree, Node *expr_node, Tree *solv_tree);

Node *DiffSin(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffCos(Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *DiffTan(Tree *expr_tree, Node *expr_node, Tree *solv_tree);

enum FuncEntryForm
{
    PREFIX,
    INFIX
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

    const FuncEntryForm life_form;                                      // PREFIX / INFIX ג נואכüםמסעט
    const FuncEntryForm tex_form;                                       // PREFIX / INFIX ג עוץו

    TreeElem_t  (*op_func)     (TreeElem_t arg1, TreeElem_t arg2);
    Node*       (*diff_func)   (Tree *expr_tree, Node *expr_node, Tree *solv_tree);
    Node *      (*simpl_nums_func) (Tree *tree, Node *cur_node);
    Node *      (*simpl_vars_func) (Tree *tree, Node *cur_node);
};


enum Operation_enum
{
    ADD,
    SUB,
    MUL,
    DIV,
    DEG,

    LN,
    LOG,

    SIN,
    COS,
    TAN,

    DIF
};

const int OPERATIONS_NUM = 11;

const Operation Operations[OPERATIONS_NUM] = 
{                                                              // life    tex
    { .num = ADD, .symbol = "+",   .tex_code = "+",      BINARY, INFIX,  INFIX,  .op_func = Add, .diff_func = DiffAdd, .simpl_nums_func = SimplNumsAdd,  .simpl_vars_func = SimplVarsAdd  },
    { .num = SUB, .symbol = "-",   .tex_code = "-",      BINARY, INFIX,  INFIX,  .op_func = Sub, .diff_func = DiffSub, .simpl_nums_func = SimplNumsSub,  .simpl_vars_func = SimplVarsArgs },
    { .num = MUL, .symbol = "*",   .tex_code = "\\cdot", BINARY, INFIX,  INFIX,  .op_func = Mul, .diff_func = DiffMul, .simpl_nums_func = SimplNumsMul,  .simpl_vars_func = SimplVarsMul  },
    { .num = DIV, .symbol = "/",   .tex_code = "\\frac", BINARY, INFIX,  PREFIX, .op_func = Div, .diff_func = DiffDiv, .simpl_nums_func = SimplNumsDiv,  .simpl_vars_func = SimplVarsArgs },
    { .num = DEG, .symbol = "^",   .tex_code = "^",      BINARY, INFIX,  INFIX,  .op_func = Deg, .diff_func = DiffDeg, .simpl_nums_func = SimplNumsDeg,  .simpl_vars_func = SimplVarsArgs },

    { .num = LN,  .symbol = "ln",  .tex_code = "\\ln",   UNARY,  PREFIX, PREFIX, .op_func = Ln,  .diff_func = DiffLn,  .simpl_nums_func = SimplNumsArgs, .simpl_vars_func = SimplVarsArgs },
    { .num = LOG, .symbol = "log", .tex_code = "\\log_", BINARY, PREFIX, PREFIX, .op_func = Log, .diff_func = DiffLog, .simpl_nums_func = SimplNumsArgs, .simpl_vars_func = SimplVarsArgs },

    { .num = SIN, .symbol = "sin", .tex_code = "\\sin",  UNARY,  PREFIX, PREFIX, .op_func = Sin, .diff_func = DiffSin, .simpl_nums_func = SimplNumsArgs, .simpl_vars_func = SimplVarsArgs },
    { .num = COS, .symbol = "cos", .tex_code = "\\cos",  UNARY,  PREFIX, PREFIX, .op_func = Cos, .diff_func = DiffCos, .simpl_nums_func = SimplNumsArgs, .simpl_vars_func = SimplVarsArgs },
    { .num = TAN, .symbol = "tan", .tex_code = "\\tan",  UNARY,  PREFIX, PREFIX, .op_func = Tan, .diff_func = DiffTan, .simpl_nums_func = SimplNumsArgs, .simpl_vars_func = SimplVarsArgs },

    { .num = DIF, .symbol = "d",   .tex_code = "\\dd",   UNARY,  PREFIX, PREFIX, .op_func = NULL }
};

const Operation *GetOperationByNode   (Node *node);
const Operation *GetOperationByNum    (int num);
const Operation *GetOperationBySymbol (char *sym);

#endif