#include <stdio.h>

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
        if (expr_node->value == ADD)
        {
            Node *left  = TakeDifferential(expr_tree, expr_node->left,  solv_tree);
            Node *right = TakeDifferential(expr_tree, expr_node->right, solv_tree);

            return NewNode(solv_tree, OP, ADD, left, right);
        }
    }
}