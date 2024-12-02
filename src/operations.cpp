#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "operations.h"
#include "derivative.h"

const Operation *GetOperationByNode(Node *node)
{
    assert(node);

    if (node->type != OP)
        return NULL;

    for (size_t i = 0; i < OPERATIONS_NUM; i++)
    {
        if ((int) node->value == Operations[i].num)
            return &Operations[i];
    }

    return NULL;
}

const Operation *GetOperationByNum(int num)
{
    for (size_t i = 0; i < OPERATIONS_NUM; i++)
    {
        if (num == Operations[i].num)
            return &Operations[i];
    }

    return NULL;
}

const Operation *GetOperationBySymbol(char *sym)
{
    assert(sym);

    for (size_t i = 0; i < OPERATIONS_NUM; i++)
    {
        if (strcmp(sym, Operations[i].symbol) == 0)
            return &Operations[i];
    }

    return NULL;
}

TreeElem_t Add(TreeElem_t arg1, TreeElem_t arg2)
{
    return arg1 + arg2;
}

TreeElem_t Sub(TreeElem_t arg1, TreeElem_t arg2)
{
    return arg1 - arg2;
}

TreeElem_t Mul(TreeElem_t arg1, TreeElem_t arg2)
{
    return arg1 * arg2;
}

TreeElem_t Div(TreeElem_t arg1, TreeElem_t arg2)
{
    return arg1 / arg2;
}

TreeElem_t Deg(TreeElem_t arg1, TreeElem_t arg2)
{
    return (TreeElem_t) pow(arg1, arg2);
}

TreeElem_t Sin(TreeElem_t arg1, TreeElem_t arg2)
{
    assert(arg1 == arg2);
    return (TreeElem_t) sin(arg1);
}

TreeElem_t Cos(TreeElem_t arg1, TreeElem_t arg2)
{
    assert(arg1 == arg2);
    return (TreeElem_t) cos(arg1);
}

TreeElem_t Tan(TreeElem_t arg1, TreeElem_t arg2)
{
    assert(arg1 == arg2);
    return (TreeElem_t) tan(arg1);
}

TreeElem_t Ln(TreeElem_t arg1, TreeElem_t arg2)
{
    assert(arg1 == arg2);
    return (TreeElem_t) log(arg1);
}

TreeElem_t Log(TreeElem_t arg1, TreeElem_t arg2)
{
    return (TreeElem_t) (log(arg2) / log(arg1));
}

Node *DiffAdd(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    Node *left  = TakeDifferential(expr_tree, expr_node->left,  solv_tree);
    Node *right = TakeDifferential(expr_tree, expr_node->right, solv_tree);

    return NewNode(solv_tree, OP, ADD, left, right);
}

Node *DiffSub(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    Node *left  = TakeDifferential(expr_tree, expr_node->left,  solv_tree);
    Node *right = TakeDifferential(expr_tree, expr_node->right, solv_tree);

    return NewNode(solv_tree, OP, SUB, left, right);
}

Node *DiffMul(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    bool left_arg_const_var  = SubtreeContainsVar(expr_node->left);
    bool right_arg_const_var = SubtreeContainsVar(expr_node->right);

    if (!left_arg_const_var && !right_arg_const_var)
    {
        return NewNode(solv_tree, NUM, 0, NULL, NULL);
    }

    else if (left_arg_const_var xor right_arg_const_var)     // один из множителей - константа
    {
        Node *num_node = NULL;
        Node *var_node = NULL;

        if (left_arg_const_var)
        {
            var_node = expr_node->left;
            num_node = TreeCopyPaste(expr_tree, solv_tree, expr_node->right);
        }
        
        else
        {
            var_node = expr_node->right;
            num_node = TreeCopyPaste(expr_tree, solv_tree, expr_node->left);
        }

        Node *var_node_diff = TakeDifferential(expr_tree, var_node, solv_tree);

        return NewNode(solv_tree, OP, MUL, num_node, var_node_diff);
    }

    Node *add_node  = NewNode(solv_tree, OP, ADD, NULL, NULL);

    Node *left_mul  = NewNode(solv_tree, OP, MUL, NULL, NULL);
    Node *right_mul = NewNode(solv_tree, OP, MUL, NULL, NULL);

    left_mul->left   = TakeDifferential(expr_tree, expr_node->left,  solv_tree);
    right_mul->right = TakeDifferential(expr_tree, expr_node->right, solv_tree);

    left_mul->right = TreeCopyPaste(expr_tree, solv_tree, expr_node->right);
    right_mul->left = TreeCopyPaste(expr_tree, solv_tree, expr_node->left);

    add_node->left = left_mul;
    add_node->right = right_mul;

    return add_node;
}

Node *DiffDiv(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    Node *left_mul  = NewNode(solv_tree, OP, MUL, NULL, NULL);
    Node *right_mul = NewNode(solv_tree, OP, MUL, NULL, NULL);

    Node *solv_numerator = NewNode(solv_tree, OP, SUB, left_mul, right_mul);

    left_mul->left   = TakeDifferential(expr_tree, expr_node->left,  solv_tree);
    right_mul->right = TakeDifferential(expr_tree, expr_node->right, solv_tree);

    right_mul->left = TreeCopyPaste(expr_tree, solv_tree, expr_node->left);
    left_mul->right = TreeCopyPaste(expr_tree, solv_tree, expr_node->right);

    Node *degree = NewNode(solv_tree, NUM, 2, NULL, NULL);
    Node *expr_denominator = TreeCopyPaste(expr_tree, solv_tree, expr_node->right);

    Node *solv_denominator = NewNode(solv_tree, OP, DEG, expr_denominator, degree);

    return NewNode(solv_tree, OP, DIV, solv_numerator, solv_denominator);
}


Node *DiffDeg(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    Node *degree = expr_node->right;
    Node *basis = expr_node->left;

    bool basis_cont_var  = SubtreeContainsVar(basis);
    bool degree_cont_var = SubtreeContainsVar(degree);

    if (basis_cont_var && degree_cont_var)                // f(x) ^ g(x)
    {
        Node *expr_cpy = TreeCopyPaste(expr_tree, solv_tree, expr_node);

        Tree tmp_expr_tree = {};
        fprintf(stderr, "ctor\n");
        TreeCtor(&tmp_expr_tree, START_TREE_SIZE ON_DIFF_DEBUG(, "tmp_expr_deg_diff"));

        Node *tmp_degree_cpy = TreeCopyPaste(expr_tree, &tmp_expr_tree, degree);
        Node *tmp_basis_cpy  = TreeCopyPaste(expr_tree, &tmp_expr_tree, basis);

        Node *tmp_ln_of_basis = NewNode(&tmp_expr_tree, OP, LN, tmp_basis_cpy, tmp_basis_cpy);

        Node *res_tmp_expr_tree = NewNode(&tmp_expr_tree, OP, MUL, tmp_degree_cpy, tmp_ln_of_basis);

        Node *arg_diff = TakeDifferential(&tmp_expr_tree, res_tmp_expr_tree, solv_tree);

        TreeDtor(&tmp_expr_tree);

        return NewNode(solv_tree, OP, MUL, expr_cpy, arg_diff);      
    }

    else if (basis_cont_var && !degree_cont_var)          // f(x) ^ a
    {
        Node *basis_cpy = TreeCopyPaste(expr_tree, solv_tree, basis);

        Node *degree_cpy = TreeCopyPaste(expr_tree, solv_tree, degree);

        Node *deg_sub_one = NewNode(solv_tree, OP, SUB, degree_cpy, NewNode(solv_tree, NUM, 1, NULL, NULL));

        Node *func_diff = NewNode(solv_tree, OP, MUL, deg_sub_one, basis_cpy);
        Node *basis_diff = TakeDifferential(expr_tree, basis, solv_tree);

        return NewNode(solv_tree, OP, MUL, func_diff, basis_diff);
    }

    else if (!basis_cont_var && degree_cont_var)         // a ^ g(x)
    {
        Node *expr_cpy   = TreeCopyPaste(expr_tree, solv_tree, expr_node);
        Node *degree_cpy = TreeCopyPaste(expr_tree, solv_tree, degree);

        Node *ln_of_deg  = NewNode(solv_tree, OP, LN, degree_cpy, degree_cpy);

        return NewNode(solv_tree, OP, MUL, expr_cpy, ln_of_deg);
    }


    else                                                                        // a ^ b
    {
        return NewNode(solv_tree, NUM, 0, NULL, NULL);
    }
}


Node *DiffSin(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    Node *arg = expr_node->left;
    Node *arg_cpy  = TreeCopyPaste(expr_tree, solv_tree, arg);
    Node *diff_sin = NewNode(solv_tree, OP, COS, arg_cpy, arg_cpy);

    Node *diff_arg = TakeDifferential(expr_tree, arg, solv_tree);

    return NewNode(solv_tree, OP, MUL, diff_sin, diff_arg);
}

Node *DiffCos(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    Node *arg = expr_node->left;
    Node *arg_cpy  = TreeCopyPaste(expr_tree, solv_tree, arg);
    Node *diff_cos = NewNode(solv_tree, OP, SIN, arg_cpy, arg_cpy);

    Node *minus_diff_cos = NewNode(solv_tree, OP, MUL, diff_cos, NewNode(solv_tree, NUM, -1, NULL, NULL));

    Node *diff_arg = TakeDifferential(expr_tree, arg, solv_tree);

    return NewNode(solv_tree, OP, MUL, minus_diff_cos, diff_arg);
}

Node *DiffTan(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    Node *arg = expr_node->left;
    Node *arg_cpy = TreeCopyPaste(expr_tree, solv_tree, arg);

    Node *cos_node = NewNode(solv_tree, OP, COS, arg_cpy, arg_cpy);

    Node *degree = NewNode(solv_tree, NUM, 2, NULL, NULL);
    Node *solv_denominator = NewNode(solv_tree, OP, DEG, cos_node, degree);
    Node *solv_numerator   = NewNode(solv_tree, NUM, 1, NULL, NULL);

    Node *diff_tan = NewNode(solv_tree, OP, DIV, solv_numerator, solv_denominator);
    Node *diff_arg = TakeDifferential(expr_tree, arg, solv_tree);

    return NewNode(solv_tree, OP, MUL, diff_tan, diff_arg);
}

Node *DiffLn(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    Node *arg = expr_node->left;
    Node *arg_cpy = TreeCopyPaste(expr_tree, solv_tree, arg);

    Node *numerator =  NewNode(solv_tree, NUM, 1, NULL, NULL);
    Node *diff_ln = NewNode(solv_tree, OP, DIV, numerator, arg_cpy);

    Node *diff_arg = TakeDifferential(expr_tree, arg, solv_tree);

    return NewNode(solv_tree, OP, MUL, diff_ln, diff_arg);
}

Node *DiffLog(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    Node *degree = expr_node->left;
    Node *arg    = expr_node->right;

    Node *arg_cpy    = TreeCopyPaste(expr_tree, solv_tree, arg);
    Node *degree_cpy = TreeCopyPaste(expr_tree, solv_tree, degree);

    
    if (!SubtreeContainsVar(degree))
    {
        Node *ln_of_degree = NewNode(solv_tree, OP, LN,  degree_cpy, degree_cpy);
        Node *denominator  = NewNode(solv_tree, OP, MUL, arg_cpy, ln_of_degree);

        Node *fraction     = NewNode(solv_tree, OP, DIV, NewNode(solv_tree, NUM, 1, NULL, NULL), denominator);

        Node *degree_diff = TakeDifferential(expr_tree, degree, solv_tree);

        return NewNode(solv_tree, OP, MUL, fraction, degree_diff);
    }

    else
    {
        Tree tmp_expr_tree = {};
        fprintf(stderr, "ctor\n");
        TreeCtor(&tmp_expr_tree, START_TREE_SIZE ON_DIFF_DEBUG(, "tmp_expr_deg_log"));

        Node *tmp_arg_cpy    = TreeCopyPaste(expr_tree, &tmp_expr_tree, arg); 
        Node *tmp_degree_cpy = TreeCopyPaste(expr_tree, &tmp_expr_tree, degree);

        Node *tmp_expr_numerator   = NewNode(&tmp_expr_tree, OP, LN, tmp_arg_cpy,    tmp_arg_cpy);
        Node *tmp_expr_denominator = NewNode(&tmp_expr_tree, OP, LN, tmp_degree_cpy, tmp_degree_cpy);

        Node *tmp_expr = NewNode(&tmp_expr_tree, OP, DIV, tmp_expr_numerator, tmp_expr_denominator);

        Node *res_diff = TakeDifferential(&tmp_expr_tree, tmp_expr, solv_tree);

        TreeDtor(&tmp_expr_tree);

        return res_diff;
    }
}