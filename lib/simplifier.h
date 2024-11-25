#ifndef SIMPLIFIER_H
#define SIMPLIFIER_H

#include "diff_tree.h"
#include "operations.h"

Node *SimplifyConstants (Tree *tree, Node *cur_node);
Node *SimplifyFraction  (Tree *tree, Node *op_node, Node *numerator, Node *denominator);

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#endif
