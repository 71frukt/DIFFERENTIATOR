#ifndef DERIVATIVE_H
#define DERIVATIVE_H

#include "diff_tree.h"

Node *TakeDerivative     (Tree *expr_tree, Node *expr_node, Tree *solv_tree);
Node *TakeHighDerivative (Tree *expr_tree, Node *start_node, Tree *diff_tree, size_t order);
Node *CalculateTailor    (Tree *expr_tree, Node *start_node, Tree *tailor_tree, size_t order);
void  PutNumInsteadVar   (Tree *tree, TreeElem_t num_val);
TreeElem_t Factorial     (TreeElem_t n);

#endif