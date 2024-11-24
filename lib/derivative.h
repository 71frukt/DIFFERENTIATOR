#ifndef DERIVATIVE_H
#define DERIVATIVE_H

#include "diff_tree.h"

Node *TakeDifferential(Tree *expr_tree, Node *expr_node, Tree *solv_tree);

#endif