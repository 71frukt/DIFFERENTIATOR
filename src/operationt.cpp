#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "operations.h"

const Operation *GetOperationByNum(int num)
{
    for (size_t i = 0; i < OPERATIONS_NUM; i++)
    {
        if (num == Operations[i].num)
            return &Operations[i];
    }

    fprintf(stderr, "unknown operation in GetOperationByNum() numbered %d\n", num);
    return NULL;
}

const Operation *GetOperationBySymbol(char *sym)
{
    assert(sym);

    for (size_t i = 0; i < OPERATIONS_NUM; i++)
    {
        if (strcmp(sym, Operations[i].symbol) == 0)
            return &Operations[i];
    }

    // fprintf(stderr, "unknown operation in GetOperationBySym() = '%s'\n", sym);
    return NULL;
}


TreeElem_t Add(TreeElem_t arg1, TreeElem_t arg2)
{
    return arg1 + arg2;
}

TreeElem_t Sub(TreeElem_t arg1, TreeElem_t arg2)
{
    return arg1 - arg2;
}

TreeElem_t Mul(TreeElem_t arg1, TreeElem_t arg2)
{
    return arg1 * arg2;
}

TreeElem_t Div(TreeElem_t arg1, TreeElem_t arg2)
{
    return arg1 / arg2;
}

TreeElem_t Deg(TreeElem_t arg1, TreeElem_t arg2)
{
    return arg1 ^ arg2;
}

TreeElem_t Sin(TreeElem_t arg1, TreeElem_t arg2)
{
    assert(arg2 == POISON_VAL);
    return (TreeElem_t) sin(arg1);
}

TreeElem_t Tan(TreeElem_t arg1, TreeElem_t arg2)
{
    assert(arg2 == POISON_VAL);
    return (TreeElem_t) tan(arg1);
}