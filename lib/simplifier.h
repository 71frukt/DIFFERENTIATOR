#ifndef SIMPLIFIER_H
#define SIMPLIFIER_H

#include "diff_tree.h"

void  SimplifyExpr        (Tree *expr_tree);
Node *SimplifyConstants   (Tree *tree, Node *cur_node);

Node *SimplNumsAdd(Tree *tree, Node *cur_node);
Node *SimplNumsSub(Tree *tree, Node *cur_node);
Node *SimplNumsMul(Tree *tree, Node *cur_node);
Node *SimplNumsDiv(Tree *tree, Node *add_node);
Node *SimplNumsDeg(Tree *tree, Node *deg_node);

Node *SimplVarsAdd(Tree *tree, Node *sub_node);

Node *SimplifyFraction    (Tree *tree, Node *op_node, Node *numerator, Node *denominator);
bool  MulByFraction       (Node *mul_node, Node *left_arg, Node *right_arg);
Node *AddFractions        (Tree *tree, Node *add_node);                         // (a / b) + (c / d)
Node *FracPlusNum         (Tree *tree, Node *add_node);
Node *ComplexToTheRight   (Node *cur_node);
void  FlipDenominator     (Node *fraction_node);
void  ExpandBrackets      (Tree *tree, Node *mul_node);
Node *RevealDoubleDeg     (Node *deg_node);

Node *TakeOutConstsInAdd  (Tree *tree, Node *cur_node);                         // (a + x) + b  =>  (a + b) + x
Node *TakeOutConstsInMul  (Tree *tree, Node *cur_node);                         // a * (b * x)  =>  (a * b) * x
Node *TakeOutConstsInDiv  (Tree *tree, Node *cur_node);                         // (a * x) / b  =>  (a / b) * x 

void  SubToAdd            (Tree *tree);                                         // 8 - f => 8 + (-f)
void  AddToSub            (Tree *tree);                                         // 8 + (-1) * f  =>  8 - f
void  VarsToGeneralform   (Tree *tree);
void  VarsToNormalForm    (Tree *tree);

Node *FractionBecomesZero (Tree *tree, Node *div_node);
bool  IsComplex           (Node *node);
bool  IsSimpleFraction    (Node *node);
bool  IsSimpleVarMember   (Node *node);                                         // example (-4) * x ^(7/9)


TreeElem_t CalculateOp    (Tree *tree, Node *op_node);
void       SimplifyArgs   (Tree *tree, Node *op_node);


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
    TreeElem_t  value;
    Node       *node;
};


struct VarMemberInfo    
{
    bool is_simple_var_member;      // 5 * x^(3 / 2)

    MemberType member_type;

    simple_ratio degree;
    simple_ratio multiplier;
};


#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define IS_INT_VAL(node)  (((long long int) node->value == node->value) && (node->type == NUM))

#endif