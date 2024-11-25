#include <stdio.h>
#include <assert.h>

#include "simplifier.h"
#include "operations.h"
#include "diff_tree.h"

Node *SimplifyConstants(Tree *tree, Node *cur_node)
{
    assert(cur_node);

    if (cur_node->type == NUM || cur_node->type == VAR)
        return cur_node;

    else
    {
        Node *left  = SimplifyConstants(tree, cur_node->left);
        Node *right = SimplifyConstants(tree, cur_node->right);

        if (left->type == NUM && right->type == NUM)
        {
            const Operation *cur_op = GetOperationByNum((int) cur_node->value);
            assert(cur_op);

            if (cur_op->num == ADD || cur_op->num == SUB || cur_op->num == MUL)
            {
                TreeElem_t new_val = cur_op->op_func(left->value, right->value);

                cur_node->left  = NULL;
                cur_node->right = NULL;

                cur_node->type = NUM;
                cur_node->value = new_val;

                RemoveNode(tree, left);
                RemoveNode(tree, right);
            }

            else if (cur_op->num == DIV)
            {
                SimplifyFraction(tree, cur_node, left, right);
            }
        }

        else if (cur_node->value == MUL)                        // количество вершин сохраняется
        {
            bool left_is_fraction  = (left->type == OP  && left->value  == DIV);
            bool right_is_fraction = (right->type == OP && right->value == DIV);

            if (left_is_fraction && (!right_is_fraction && right->type == NUM))
            {
                cur_node->value = DIV;
                cur_node->left->value = MUL;

                TreeElem_t tmp_val = right->value;

                right->value = left->right->value;
                left->right->value = tmp_val;
            }

            else if (right_is_fraction && (!left_is_fraction && left->type == NUM))
            {
                cur_node->value = DIV;
                cur_node->right->value = MUL;

                TreeElem_t tmp_val = left->value;

                left->value = right->right->value;
                right->right->value = tmp_val;

                cur_node->left  = right;
                cur_node->right = left;
            }

            else if (left_is_fraction && right_is_fraction)
            {
                cur_node->value = DIV;
                left->value     = MUL;
                right->value    = MUL;

                TreeElem_t tmp_for_numerator = left->right->value;
                left->right->value = right->left->value;
                right->left->value = tmp_for_numerator;
            }
        }        

        return cur_node;
    }
}

Node *SimplifyFraction(Tree *tree, Node *op_node, Node *numerator, Node *denominator)   // TODO: удаление старых узлов
{
    bool numerator_is_int   = ((int) numerator->value   == numerator->value);
    bool denominator_is_int = ((int) denominator->value == denominator->value);

    if ((!numerator_is_int || !denominator_is_int) || 
        ((numerator_is_int && denominator_is_int) && (numerator->value % denominator->value) == 0))         // числитель или знаменатель не целые или делятся нацело
    {
        TreeElem_t new_val = numerator->value / denominator->value;

        op_node->left  = NULL;
        op_node->right = NULL;

        op_node->type = NUM;
        op_node->value = new_val;
    }

    else
    {
        TreeElem_t start_val = MIN(numerator->value, denominator->value) / 2;

        for (TreeElem_t divider = start_val; divider > 0; divider--)
        {
            if (divider > numerator->value)
                divider = numerator->value;
            
            if (numerator->value % divider == 0 && denominator->value % divider == 0)
            {
                numerator->value   /= divider;
                denominator->value /= divider;
            }
        } 
    }

    return op_node;
}