#include <stdio.h>
#include <assert.h>

#include "simplifier.h"
#include "operations.h"
#include "diff_tree.h"
#include "diff_debug.h"

extern FILE *LogFile;

void SimplifyExpr(Tree *expr_tree)
{
    fprintf(stderr, "rabotaet yproshalka\n");
    DIFF_DUMP(expr_tree);

    SubToAdd(expr_tree);

    TakeOutConsts(expr_tree, expr_tree->root_ptr);

    SimplifyConstants(expr_tree, expr_tree->root_ptr);

    AddToSub(expr_tree);

    DIFF_DUMP(expr_tree);
}

Node *SimplifyConstants(Tree *tree, Node *cur_node)
{
    assert(tree);
    assert(cur_node);


// fprintf(stderr, "type = %d, val = %d\n", cur_node->type, cur_node->value);

    if (cur_node->type == NUM || cur_node->type == VAR)
        return cur_node;

    cur_node->left  = SimplifyConstants(tree, cur_node->left);
    cur_node->right = SimplifyConstants(tree, cur_node->right);

    if (cur_node->left->type == NUM && cur_node->right->type == NUM)
    {
        const Operation *cur_op = GetOperationByNum((int) cur_node->value);
        assert(cur_op);

        if (cur_op->num == ADD || cur_op->num == SUB || cur_op->num == MUL || (cur_op->num == DEG && CanCalcDeg(cur_node->right)))
        {
            TreeElem_t new_val = cur_op->op_func(cur_node->left->value, cur_node->right->value);

            RemoveNode(tree, &cur_node->left);
            RemoveNode(tree, &cur_node->right);

            cur_node->left  = NULL;
            cur_node->right = NULL;

            cur_node->type = NUM;
            cur_node->value = new_val;

            return cur_node;
        }
    }


    if (OpNodeIsCommutativity(cur_node))
        ComplexToTheRight(cur_node);

    if (cur_node->value == DIV)
    {
    DIFF_DUMP(tree);


        // fprintf(stderr, "in div\n");

        TakeOutConsts(tree, cur_node);

        // fprintf(stderr, "after take out consts\n");

        if (cur_node->value == DIV)
            SimplifyFraction(tree, cur_node, cur_node->left, cur_node->right);
    }

    else if (cur_node->value == MUL)
    {
    DIFF_DUMP(tree);


        fprintf(stderr, "in mul\n");
        fprintf(LogFile, "in mul\n");

        TakeOutConsts(tree, cur_node);                                                          // a * (b * x)  =>  (a * b) * x

        if (MulByFraction(cur_node, cur_node->left, cur_node->right))                           // если есть умножение на дробь
            SimplifyConstants(tree, cur_node);

        bool is_still_mul = (cur_node->type == OP && cur_node->value == MUL);

        fprintf(stderr, "is still mul  %d\n", is_still_mul);

        if (is_still_mul)
        {
            bool left_is_simple_multiplier = !SubtreeContComplicOperation(cur_node->left);
            bool right_is_sum = (cur_node->right->type == OP && cur_node->right->value == ADD);

            if (left_is_simple_multiplier && right_is_sum)     // раскрыть скобки
            {
                fprintf(stderr, "ExpandBrackets\n");

                ExpandBrackets(tree, cur_node);
                SimplifyConstants(tree, cur_node);
            }
        }
    }        

    // fprintf(stderr, "end of simpl\n");
    return cur_node;
}

Node *SimplifyFraction(Tree *tree, Node *op_node, Node *numerator, Node *denominator)
{
    assert(tree);
    assert(numerator);
    assert(denominator);

    // fprintf(stderr, "in simpl frac\n");

    if (numerator->type == NUM && numerator->value == 0)
        return FractionBecomesZero(tree, op_node);

    bool numerator_is_int   = IS_INT_VAL(numerator);
    bool denominator_is_int = IS_INT_VAL(denominator);

    bool can_calc = ((!numerator_is_int || !denominator_is_int) && (numerator->type == NUM && denominator->type == NUM)) ||                                                               // они double
                     ((numerator_is_int &&  denominator_is_int) && (numerator->value % denominator->value) == 0);                // или делятся нацело

    if (numerator->type == NUM && denominator->type == NUM && can_calc)
    {
        TreeElem_t new_val = numerator->value / denominator->value;
        
        RemoveNode(tree, &op_node->left);
        RemoveNode(tree, &op_node->right);

        op_node->left  = NULL;
        op_node->right = NULL;

        op_node->type = NUM;
        op_node->value = new_val;
    }

    else if ((numerator->type   == OP && numerator->value   == DIV)
          || (denominator->type == OP && denominator->value == DIV))
    {
        DIFF_DUMP(tree);
        FlipDenominator(op_node);
        DIFF_DUMP(tree);
        SimplifyConstants(tree, op_node);
    }

    else if (numerator_is_int && denominator_is_int)   // и числитель, и знаменатель целые
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

void FlipDenominator(Node *fraction_node)           // вызывается когда в числителе или в знаменателе дробь
{
    assert(fraction_node);
    assert(fraction_node->right);
    assert(fraction_node->right);

    fprintf(LogFile, "\nFlipDenominator()\n\n");

    Node *denom = fraction_node->right;

    if (denom->type == OP && denom->value == DIV)   // в знаменателе дробь
    {
        fprintf(stderr, "v znamematele drob\n");
        fprintf(LogFile, "\nv znamematele drob\n");

        Node *numerator_of_denom = denom->left;

        denom->value = MUL;
        denom->left = fraction_node->left;

        fraction_node->left  = denom;
        fraction_node->right = numerator_of_denom;
    }

    else                                            // в знаменателе не дробь => в числителе дробь
    {

        fprintf(stderr, "\nv chislitele drob\n");
        fprintf(LogFile, "\nv chislitele drob\n");

        Node *prev_denom = fraction_node->right;

        fraction_node->right = fraction_node->left;
        fraction_node->right->value = MUL;

        Node *new_denom = fraction_node->right;

        fraction_node->left  = fraction_node->left->left;

        new_denom->left  = new_denom->right;
        new_denom->right = prev_denom;
    }
}

bool MulByFraction(Node *mul_node, Node *left_arg, Node *right_arg)
{
    assert(mul_node);
    assert(left_arg);
    assert(right_arg);

    fprintf(stderr, "in mul frac\n");

    fprintf(LogFile, "\n\nIN MUL FRAC\n\n");

    bool exists_mul_by_frac = true;

    bool left_is_fraction  = (left_arg->type  == OP && left_arg->value  == DIV);
    bool right_is_fraction = (right_arg->type == OP && right_arg->value == DIV);

    bool left_is_complex  = IsComplex(left_arg);
    bool right_is_complex = IsComplex(right_arg);
    fprintf(stderr, "in mul frac 2: left_complex = %d, right_complex = %d\n leftfrac = %d, rightfrac = %d\n", left_is_complex, right_is_complex, left_is_fraction, right_is_fraction);

    if (left_is_fraction && !right_is_fraction && !right_is_complex)
    {
        mul_node->value = DIV;
        mul_node->left->value = MUL;

        mul_node->right = left_arg->right;
        left_arg->right = right_arg;
    }

    else if (right_is_fraction && !left_is_fraction && !left_is_complex)
    {
        mul_node->value = DIV;

        mul_node->left = right_arg;
        mul_node->left->value = MUL;

        mul_node->right = right_arg->right;
        right_arg->right = left_arg;
    }

    else if (left_is_fraction && right_is_fraction)
    {
        fprintf(stderr, "mark 1\n");

        mul_node->value  = DIV;
        left_arg->value  = MUL;
        right_arg->value = MUL;

        Node *tmp_node = left_arg->right;

        left_arg->right = right_arg->left;
        right_arg->left = tmp_node;
    }

    else
        exists_mul_by_frac = false;

    fprintf(stderr, "end of mul frac, exists mul = %d\n", exists_mul_by_frac);
    fprintf(LogFile, "end of mul frac, exists mul = %d\n", exists_mul_by_frac);

    return exists_mul_by_frac;
}

Node *ComplexToTheRight(Node *cur_node)
{
    assert(cur_node);
    assert(cur_node->left);
    assert(cur_node->right);
    assert(OpNodeIsCommutativity(cur_node));

    if (IsComplex(cur_node->left) && !IsComplex(cur_node->right))
    {
        Node *tmp_node  = cur_node->left;
        cur_node->left  = cur_node->right;
        cur_node->right = tmp_node;
    }

    return cur_node;
}

Node *TakeOutConsts(Tree *tree, Node *cur_node)                     // a * (b * x)  =>  (a * b) * x
{                                                                   // (a * x) / b  =>  (a / b) * x
    bool is_suitable_op = (cur_node->type == OP && (cur_node->value == MUL || cur_node->value == DIV));

    if (cur_node == NULL || !is_suitable_op)
        return cur_node;

    if (cur_node->value == MUL)
        ComplexToTheRight(cur_node);

    Node *left_arg  = TakeOutConsts(tree, cur_node->left);
    Node *right_arg = TakeOutConsts(tree, cur_node->right); 

    if (cur_node->value == MUL)
    {
        if (right_arg->type != OP || right_arg->value != MUL)
            return cur_node;

        if (!IsComplex(left_arg)        && IsComplex(right_arg) &&
            !IsComplex(right_arg->left) && IsComplex(right_arg->right))
        {
            cur_node->right = right_arg->right;
            cur_node->left  = right_arg;

            right_arg->right = left_arg;
        }
    }

    else
    {
        if (left_arg->type != OP || left_arg->value != MUL)
            return cur_node;

        if (IsComplex(left_arg)       && !IsComplex(right_arg) && 
           !IsComplex(left_arg->left) && IsComplex(left_arg->right))
        {
            cur_node->right = left_arg->right;
            left_arg->right = right_arg;

            left_arg->value = DIV;
            cur_node->value = MUL;

            SimplifyConstants(tree, cur_node);
        }
    }

    assert(cur_node->type == OP && cur_node->value == MUL);
    return cur_node;
}

bool CanCalcDeg(Node *degree_node)
{
    return ((IS_INT_TREE && IS_INT_VAL(degree_node) && degree_node->value > 0) || !IS_INT_TREE);
}

Node *FractionBecomesZero(Tree *tree, Node *div_node)
{
    Node *denominator = div_node->right;

    if (denominator->type == NUM && denominator->value == 0)
    {
        fprintf(stderr, "%p\n", div_node);

        fprintf(stderr, "Warning: uncertainty type of '0/0'\n");
        return div_node;
    }

    RemoveNode(tree, &div_node->left);
    RemoveNode(tree, &div_node->right);

    div_node->left  = NULL;
    div_node->right = NULL;

    div_node->type  = NUM;
    div_node->value = 0;

    return div_node;
}

void SubToAdd(Tree *tree)                                           // 8 - x  =>  8 + (-1) * x
{
    assert(tree);

    for (size_t i = 0; i < tree->size; i++)
    {
        Node *cur_node = tree->node_ptrs[i];

        if (cur_node == NULL)
            continue;

        if (cur_node->type == OP && cur_node->value == SUB)
        {
            Node *minus = NewNode(tree, NUM, -1, NULL, NULL);
            cur_node->right = NewNode(tree, OP, MUL, minus, cur_node->right);
            cur_node->value = ADD;
        }
    }
}

void AddToSub(Tree *tree)                                       // 8 + (-1) * x  =>  8 - x
{
    assert(tree);

    for (size_t i = 0; i < tree->size; i++)
    {
        Node *cur_node = tree->node_ptrs[i];

        if (cur_node == NULL)
            continue;
        
        if (cur_node->type == OP && cur_node->value == ADD)
        {
            Node *arg_2 = cur_node->right;

            if (arg_2->type == OP && arg_2->value == MUL && arg_2->left->type == NUM && arg_2->left->value == -1)
            {

                cur_node->value = SUB;
                cur_node->right = arg_2->right;

                RemoveNode(tree, &arg_2->left);
                RemoveNode(tree, &arg_2);
            }
        }
    }
}

void ExpandBrackets(Tree *tree, Node *mul_node)
{
    assert(tree);
    assert(mul_node);

    fprintf(stderr, "mul_node = %p\n", mul_node);

    Node *simple_multiplier = mul_node->left;
    Node *simple_multiplier_cpy = TreeCopyPaste(tree, tree, simple_multiplier);

    Node *left_mul = NewNode(tree, OP, MUL, simple_multiplier, mul_node->right->left);

    Node *right_mul = mul_node->right;
    right_mul->value = MUL;

    right_mul->left = simple_multiplier_cpy;
    // right_mul->right остаётся тем же, что и был до этого

    mul_node->value = ADD;
    mul_node->left = left_mul;
    // mul_node->right остаётся тем же, что и был до этого

    fprintf(stderr, "end of exp brack\n");
}

bool IsComplex(Node *node)
{
    assert(node);
    return (SubtreeContainsVar(node) || SubtreeContComplicOperation(node));
}