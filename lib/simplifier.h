#ifndef SIMPLIFIER_H
#define SIMPLIFIER_H

#include "diff_tree.h"

void  SimplifyExpr      (Tree *expr_tree, Node *start_node);
Node *SimplifyConstants (Tree *tree, Node *cur_node);
Node *SimplifyVars      (Tree *tree, Node *cur_node);

Node *SimplNumsAdd      (Tree *tree, Node *cur_node);
Node *SimplNumsSub      (Tree *tree, Node *cur_node);
Node *SimplNumsMul      (Tree *tree, Node *cur_node);
Node *SimplNumsDiv      (Tree *tree, Node *add_node);
Node *SimplNumsDeg      (Tree *tree, Node *deg_node);

Node *SimplNumsOther    (Tree *tree, Node *cur_node);

Node *SimplVarsAdd      (Tree *tree, Node *sub_node);
Node *SimplVarsMul      (Tree *tree, Node *sub_node);

void  ExpandAddBrackets (Tree *tree, Node *mul_node);                           // a * (x + y)
void  ExpandDegBrackets (Tree *tree, Node *deg_op_node);                        // (f * g) ^ a

Node *MulByNull         (Tree *tree, Node *mul_node);
Node *FractionInDeg     (Tree *tree, Node *pow_node);
bool  MulByFraction     (Node *mul_node, Node *left_arg, Node *right_arg);

Node *SimplifyFraction  (Tree *tree, Node *op_node, Node *numerator, Node *denominator);
Node *AddFractions      (Tree *tree, Node *add_node);                           // (a / b) + (c / d)
Node *FracPlusNum       (Tree *tree, Node *add_node);
void  FlipDenominator   (Node *fraction_node);
Node *ComplexToTheRight (Node *cur_node);

Node *RevealDoubleDeg   (Node *deg_node);

Node *TakeOutConsts  (Tree *tree, Node *cur_node);                              // a + (b + x)   =>  (a + b) + x  или (a + f(x)) + g(x) => a + (f(x) + g(x))  и аналогично для умножения
Node *TakeOutConstsInDiv  (Tree *tree, Node *cur_node);                         // (a * x) / b  =>  (a / b) * x 

void  SubToAdd            (Tree *tree, Node *start_node);                       // 8 - x  =>  8 + (-1) * x
void  AddToSub            (Tree *tree, Node *start_node);                       // 8 + (-1) * f  =>  8 - f
void  VarsToGeneralform   (Tree *tree, Node *start_node);
void  VarsToNormalForm    (Tree *tree, Node *start_node);

Node *FractionBecomesZero (Tree *tree, Node *div_node);
bool  IsComplex           (Node *node);
bool  IsSimpleFraction    (Node *node);
bool  IsSimpleVarMember   (Node *node);                                         // example (-4) * x ^(7/9)
bool  FractionsAreEqual   (Node *div_1, Node *div_2);                           // 3/2 != 6/4


TreeElem_t  CalculateOp    (Tree *tree, Node *op_node);
Node       *SimplNumsArgs  (Tree *tree, Node *op_node);
Node       *SimplVarsArgs  (Tree *tree, Node *op_node);


enum MemberType
{
    MEMBER_NO_TYPE,
    MEMBER_VAR,
    MEMBER_VAR_DEG,
    MEMBER_MUL_VAR,
    MEMBER_MUL_VAR_DEG
};

union simple_ratio
{
    TreeElem_t  val;
    Node       *node;
};


struct VarMemberInfo    
{
    bool is_simple_var_member;      // 5 * x^(3 / 2)

    MemberType member_type;

    simple_ratio degree;
    simple_ratio multiplier;
};


#include <cmath>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

const double COMPARE_ORDER = pow(10, -3);

#define IS_INT_VAL(node)  ((node->type == NUM) && (IS_INT_TREE || (fabs(node->val.num - round(node->val.num)) < COMPARE_ORDER)))

#define ARE_EQUAL(val_1, val_2)  (fabs(val_1 - val_2) < COMPARE_ORDER)

#define CHECK_NODE_OP(node, op_num)  (node->type == OP && node->val.op == op_num)

#endif