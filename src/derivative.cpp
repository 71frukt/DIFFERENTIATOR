#include <stdio.h>

#include "operations.h"
#include "derivative.h"

Node *TakeDifferential(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    if (expr_node->type == NUM)
    {
        return NewNode(solv_tree, NUM, 0, NULL, NULL);
    }

    else if (expr_node->type == VAR)
    {
        return NewNode(solv_tree, NUM, 1, NULL, NULL);
    }

    else
    {
        const Operation *cur_op = GetOperationByNode(expr_node);

        return cur_op->diff_func(expr_tree, expr_node, solv_tree);
    }
}