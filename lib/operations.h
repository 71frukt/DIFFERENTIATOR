#ifndef OPERATIONS_H
#define OPERATIONS_H

#define ADD_SYMBOL  "+"       // TODO 1
#define SUB_SYMBOL  "-"
#define MUL_SYMBOL  "*"
#define DIV_SYMBOL  "/"
#define DEG_SYMBOL  "^"

#define ADD_TEX     "+"
#define SUB_TEX     "-"
#define MUL_TEX     "\\cdot"
#define DIV_TEX     "\\frac"
#define DEG_TEX     "^"

#define TO_STR(mark)  #mark

enum Operation
{
    ADD,
    SUB,
    MUL,
    DIV,
    DEG
};

#endif