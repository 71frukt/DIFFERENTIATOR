#ifndef SIMPLIFIER_H
#define SIMPLIFIER_H

#include "diff_tree.h"
#include "operations.h"

void  SimplifyExpr         (Tree *expr_tree);
Node *SimplifyConstants    (Tree *tree, Node *cur_node);
Node *SimplifyFraction     (Tree *tree, Node *op_node, Node *numerator, Node *denominator);
bool  MulByFraction        (Node *mul_node, Node *left_arg, Node *right_arg);
Node *VarsToTheRight       (Node *cur_node);
Node *RearrangeEdgesOfVars (Node *mul_node);                     // a * (b * x)  =>  (a * b) * x
void  FlipDenominator      (Node *fraction_node);
bool  CanCalcDeg           (TreeElem_t degree_val);


#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define IS_INT_VAL(val)  ((long long int) val == val)

#endif
