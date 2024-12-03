#include <stdio.h>

#include "operations.h"
#include "derivative.h"

Node *TakeDifferential(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    if (expr_node->type == NUM)
    {
        return NewNode(solv_tree, NUM, {.num = 0}, NULL, NULL);
    }

    else if (expr_node->type == VAR)
    {
        return NewNode(solv_tree, NUM, {.num = 1}, NULL, NULL);
    }

    else if (expr_node->type == CHANGE)
    {
        Change *cur_change = expr_node->val.change;

        Node *new_node = NewNode(solv_tree, CHANGE, {}, NULL, NULL);
        Change *diff_change = new_node->val.change;
        
        diff_change->name = cur_change->name;
        diff_change->derivative_num = cur_change->derivative_num++;
        diff_change->target_node = TakeDifferential(expr_tree, cur_change->target_node, solv_tree);

        return new_node;
    }

    else if (expr_node->type == OP)
    {
        const Operation *cur_op = GetOperationByNode(expr_node);

        return cur_op->diff_func(expr_tree, expr_node, solv_tree);
    }

    else
        return NULL;
}