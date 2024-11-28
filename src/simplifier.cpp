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
    VarsToGeneralform(expr_tree);                   // x --> 1 * x ^ 1

fprintf(LogFile, "\n\nafter VarsToGeneralform()\n\n");
    DIFF_DUMP(expr_tree);

    SimplifyConstants(expr_tree, expr_tree->root_ptr);
fprintf(LogFile, "\n\nafter SimplifyConstants()\n\n");
    DIFF_DUMP(expr_tree);


    // SimplVarsAdd(expr_tree, expr_tree->root_ptr);
fprintf(LogFile, "\n\nafter SimplVarsAdd()\n\n");
    DIFF_DUMP(expr_tree);



    VarsToNormalForm(expr_tree);
fprintf(LogFile, "\n\nafter VarsToNormalForm()\n\n");
    DIFF_DUMP(expr_tree);



    AddToSub(expr_tree);
fprintf(LogFile, "\n\nafter AddToSub()\n\n");
    DIFF_DUMP(expr_tree);


    DIFF_DUMP(expr_tree);
}

Node *SimplifyConstants(Tree *tree, Node *cur_node)
{
    assert(cur_node);

    const Operation *op  = GetOperationByNode(cur_node);

    if (op != NULL)
        cur_node = op->simpl_nums_func(tree, cur_node);

    return cur_node;
}

Node *SimplNumsAdd(Tree *tree, Node *add_node)
{
    fprintf(LogFile, "start of SimplNumsAdd()\n");
    DIFF_DUMP(tree);

    SimplifyArgs(tree, add_node);

    if (add_node->left->type == NUM && add_node->right->type == NUM)
    {
        CalculateOp(tree, add_node);
        return add_node;
    }

    if (IsSimpleFraction(add_node->left) && IsSimpleFraction(add_node->right))
    {
        Node *new_fract_node = AddFractions(tree, add_node);               // ADD --> DIV
        SimplifyConstants(tree, new_fract_node);
        return new_fract_node;
    }

    if ((IsSimpleFraction(add_node->left)  && add_node->right->type == NUM)
     || (IsSimpleFraction(add_node->right) && add_node->left->type  == NUM))
    {
        Node *new_fract_node = FracPlusNum(tree, add_node);               // ADD --> DIV
        SimplifyConstants(tree, new_fract_node);
        return new_fract_node;
    }

    ComplexToTheRight(add_node);

    TakeOutConstsInAdd(tree, add_node);              // a + (b + x)  =>  (a + b) + x
    SimplifyConstants(tree, add_node->left);

    fprintf(LogFile, "end of SimplNumsAdd()\n");
    DIFF_DUMP(tree);

    return add_node;
}

Node *SimplNumsSub(Tree *tree, Node *sub_node)
{
    SimplifyArgs(tree, sub_node);

    if (sub_node->left->type == NUM && sub_node->right->type == NUM)
    {
        CalculateOp(tree, sub_node);
        return sub_node;
    }

    ComplexToTheRight(sub_node);

    return sub_node;
}

Node *SimplNumsMul(Tree *tree, Node *mul_node)
{
    fprintf(LogFile, "start of SimplNumsMul()\n");
    DIFF_DUMP(tree);

    SimplifyArgs(tree, mul_node);

    fprintf(stderr, "after simplify args\n");

    if (mul_node->left->type == NUM && mul_node->right->type == NUM)
    {
        CalculateOp(tree, mul_node);
        return mul_node;
    }

    ComplexToTheRight(mul_node);
    TakeOutConstsInMul(tree, mul_node);              // a * (b * x)  =>  (a * b) * x

    fprintf(LogFile, "before first rec in SimplNumsMul()\n");
    DIFF_DUMP(tree);

    SimplifyConstants(tree, mul_node->left);

    if (MulByFraction(mul_node, mul_node->left, mul_node->right))                           // если есть умножение на дробь  MUL --> DIV
    {
        SimplifyConstants(tree, mul_node);
        return mul_node;
    }

    bool is_still_mul = (mul_node->type == OP && mul_node->value == MUL);   // если не свернулось в константу

    if (is_still_mul)
    {
        bool left_is_simple_multiplier = !SubtreeContComplicOperation(mul_node->left);
        bool right_is_sum = (mul_node->right->type == OP && mul_node->right->value == ADD);

        if (left_is_simple_multiplier && right_is_sum)      // раскрыть скобки
        {
            fprintf(stderr, "ExpandAddBrackets\n");

            ExpandAddBrackets(tree, mul_node);                 // MUL -> ADD
            SimplifyConstants(tree, mul_node);
            return mul_node;
        }
    }

    fprintf(LogFile, "end of SimplNumsMul()\n");
    DIFF_DUMP(tree);

    return mul_node;
}

Node *SimplNumsDiv(Tree *tree, Node *div_node)
{
    fprintf(LogFile, "start of SimplNumsDiv()\n");
    DIFF_DUMP(tree);

    SimplifyArgs(tree, div_node);

    if (TakeOutConstsInDiv(tree, div_node)->value != DIV)    // (a * x) / b  =>  (a / b) * x  --- упростилось и sub_node->type != DIV
    {
        SimplifyConstants(tree, div_node->left);
        return div_node;
    }

    if (div_node->left->type == NUM && div_node->left->value == 0)
        return FractionBecomesZero(tree, div_node);

    Node *numerator   = div_node->left;
    Node *denominator = div_node->right;

    bool numerator_is_int   = IS_INT_VAL(div_node->left);
    bool denominator_is_int = IS_INT_VAL(div_node->right);

    bool can_calc = ((!numerator_is_int || !denominator_is_int) && (numerator->type == NUM && denominator->type == NUM)) ||         // оба double                                                        // они double
                     ((numerator_is_int &&  denominator_is_int) && (numerator->value % denominator->value) == 0);                   // или делятся нацело

    if (numerator->type == NUM && denominator->type == NUM && can_calc)
    {
        CalculateOp(tree, div_node);
        return div_node;
    }

    else if ((numerator->type   == OP && numerator->value   == DIV)
          || (denominator->type == OP && denominator->value == DIV))
    {
        FlipDenominator(div_node);
        SimplifyConstants(tree, div_node);
    }

    else if (numerator_is_int && denominator_is_int)   // и числитель, и знаменатель целые
    {
        TreeElem_t start_val = MIN(abs(numerator->value), abs(denominator->value)) / 2;

        for (TreeElem_t divider = start_val; divider > 0; divider--)
        {
            if (divider > abs(numerator->value))
                divider = abs(numerator->value);


            if (numerator->value % divider == 0 && denominator->value % divider == 0)
            {
                numerator->value   /= divider;
                denominator->value /= divider;
            }
        }
    }

    if (denominator->type == NUM && denominator->value < 0)
    {
        denominator->value *= -1;

        Node *minus    = NewNode(tree, NUM, -1, NULL, NULL);
        div_node->left = NewNode(tree, OP, MUL, minus, numerator);

        SimplifyConstants(tree, div_node->left);
    }

    fprintf(LogFile, "end of SimplNumsDiv()\n");
    DIFF_DUMP(tree);

    return div_node;    
}

Node *SimplNumsDeg(Tree *tree, Node *deg_node)
{
    assert(tree);
    assert(deg_node);

    SimplifyArgs(tree, deg_node);

    Node *basis  = deg_node->left;
    Node *degree = deg_node->right;

    bool basis_is_valid  = ((IS_INT_TREE && basis->type  == NUM)                      || !IS_INT_TREE);
    bool degree_is_valid = ((IS_INT_TREE && degree->type == NUM && degree->value > 0) || !IS_INT_TREE);

    if (basis->type == NUM && basis->value == 1)
    {
        Node one = *basis;

        RemoveSubtree(tree, &basis);
        RemoveSubtree(tree, &degree);

        *deg_node = one;
        return deg_node;
    }

    if (basis_is_valid && degree_is_valid)
    {
        CalculateOp(tree, deg_node);
        return deg_node;
    }

    if (basis->type == OP && basis->value == DEG)
    {
        RevealDoubleDeg(deg_node);
        SimplifyConstants(tree, deg_node);
        return deg_node;
    }

    if (basis->type == OP && basis->value == MUL && !IsComplex(degree))
    {
        ExpandDegBrackets(tree, deg_node);      // DEG --> MUL


    fprintf(LogFile, "after ExpandDegBrackets()\n");
    DIFF_DUMP(tree);


        SimplifyConstants(tree, deg_node);
        return deg_node;
    }

    return deg_node;
}

Node *RevealDoubleDeg(Node *deg_node)
{
    assert(deg_node);

    Node *old_basis  = deg_node->left;
    Node *old_degree = deg_node->right;

    Node *new_basis   = old_basis->left;
    Node *new_degree  = old_basis;
    new_degree->value = MUL;

    new_degree->left  = old_basis->right;
    new_degree->right = old_degree;

    deg_node->left  = new_basis;
    deg_node->right = new_degree;

    return deg_node;
}

Node *SimplVarsAdd(Tree *tree, Node *sub_node)
{
    assert(tree);
    assert(sub_node);

    Node *arg_1 = sub_node->left;
    Node *arg_2 = sub_node->right;

    if (IsSimpleVarMember(arg_1) && IsSimpleVarMember(arg_2))
    {
        fprintf(stderr, "simpling vars add\n");

        Node *deg_1 = arg_1->right->right;
        Node *deg_2 = arg_2->right->right;

        Node *mult_1 = arg_1->left;
        Node *mult_2 = arg_2->left;

        Node *var_1  = arg_1->right->left;
        Node *var_2  = arg_2->right->left;

        if (IsComplex(deg_1) || IsComplex(deg_2) || IsComplex(mult_1) || IsComplex(mult_2))
        {
            fprintf(stderr, "are not complex\n");
            return sub_node;
        }

        bool degrees_are_equal = ((deg_1->type == NUM && deg_2->type == NUM && deg_1->value == deg_2->value) 
                               || (CHECK_NODE_OP(deg_1, DIV) && CHECK_NODE_OP(deg_2, DIV) && FractionsAreEqual(deg_1, deg_2)));

        if (var_1->value == var_2->value && degrees_are_equal)
        {
            TreeElem_t new_mult_val = mult_1->value + mult_2->value;

            Node *new_mult = mult_1;
            new_mult->value = new_mult_val;

            Node *res_node = sub_node;
            res_node->value = MUL;

            res_node->right = arg_1->right;
            res_node->left  = new_mult;

            RemoveNode(tree, &arg_1);
            RemoveSubtree(tree, &arg_2);
            // RemoveNode(tree, &var_2);
            // RemoveNode(tree, &deg_2);
            // RemoveNode(tree, &arg_2->right);
            // RemoveNode(tree, &mult_2);
            // RemoveNode(tree, &arg_2);

            return sub_node;
        }

        // else if (var_1->value == var_2->value && CHECK_NODE_OP(deg_1, DIV) && CHECK_NODE_OP(deg_2, DIV) && FractionsAreEqual(deg_1, deg_2))
        // {

        // }
    }

    return sub_node;
}

void SimplifyArgs(Tree *tree, Node *op_node)
{
    assert(op_node);
    assert(op_node->left);
    assert(op_node->right);

    const Operation *left_op  = GetOperationByNode(op_node->left);
    const Operation *right_op = GetOperationByNode(op_node->right);

    if (left_op != NULL)
        op_node->left = left_op->simpl_nums_func(tree, op_node->left);

    if (right_op != NULL)
        op_node->right = right_op->simpl_nums_func(tree, op_node->right);
}

TreeElem_t CalculateOp(Tree *tree, Node *op_node)
{
    assert(tree);
    assert(op_node);

    const Operation *op = GetOperationByNode(op_node);
    assert(op);

    TreeElem_t new_val = op->op_func(op_node->left->value, op_node->right->value);

    RemoveNode(tree, &op_node->left);
    RemoveNode(tree, &op_node->right);
    op_node->left  = NULL;
    op_node->right = NULL;

    op_node->type = NUM;
    op_node->value = new_val;

    return new_val;
}

Node *SimplifyFraction(Tree *tree, Node *op_node, Node *numerator, Node *denominator)
{
    assert(tree);
    assert(numerator);
    assert(denominator);

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
        TreeElem_t start_val = MIN(abs(numerator->value), abs(denominator->value)) / 2;

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

    else if (denominator->type == NUM && denominator->value < 0)
    {
        denominator->value *= -1;

        Node *minus   = NewNode(tree, NUM, -1, NULL, NULL);
        op_node->left = NewNode(tree, OP, MUL, minus, numerator);

        SimplifyConstants(tree, op_node->left);
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
        Node *numerator_of_denom = denom->left;

        denom->value = MUL;
        denom->left = fraction_node->left;

        fraction_node->left  = denom;
        fraction_node->right = numerator_of_denom;
    }

    else                                            // в знаменателе не дробь => в числителе дробь
    {
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

    bool exists_mul_by_frac = true;

    bool left_is_fraction  = (left_arg->type  == OP && left_arg->value  == DIV);
    bool right_is_fraction = (right_arg->type == OP && right_arg->value == DIV);

    bool left_is_complex  = IsComplex(left_arg);
    bool right_is_complex = IsComplex(right_arg);

    if (left_is_fraction && !right_is_fraction && !right_is_complex)
    {
        // mul_node->value = DIV;
        mul_node->left->value = MUL;

        mul_node->right = left_arg->right;
        left_arg->right = right_arg;
    }

    else if (right_is_fraction && !left_is_fraction && !left_is_complex)
    {
        // mul_node->value = DIV;

        mul_node->left = right_arg;
        mul_node->left->value = MUL;

        mul_node->right = right_arg->right;
        right_arg->right = left_arg;
    }

    else if (left_is_fraction && right_is_fraction)
    {
        left_arg->value  = MUL;
        right_arg->value = MUL;

        Node *tmp_node = left_arg->right;

        left_arg->right = right_arg->left;
        right_arg->left = tmp_node;
    }

    else
        return false;

    mul_node->value  = DIV;

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

Node *TakeOutConstsInAdd(Tree *tree, Node *cur_node)                        // (a + x) + b  =>  (a + b) + x
{
    assert(tree);
    assert(cur_node);

    Node *left_arg  = cur_node->left;
    Node *right_arg = cur_node->right; 

    if (right_arg->type != OP || right_arg->value != ADD)
        return cur_node;

    if (!IsComplex(left_arg)        && IsComplex(right_arg) &&
        !IsComplex(right_arg->left) && IsComplex(right_arg->right))
    {

        cur_node->right = right_arg->right;
        cur_node->left  = right_arg;

        right_arg->right = left_arg;
    }

    return cur_node;
}

Node *TakeOutConstsInMul(Tree *tree, Node *cur_node)                        // a * (b * x)  =>  (a * b) * x
{
    assert(tree);
    assert(cur_node);

    Node *left_arg  = cur_node->left;
    Node *right_arg = cur_node->right; 

    if (right_arg->type != OP || right_arg->value != MUL)
            return cur_node;

    fprintf(stderr, "\n\n\ncur_node->left = %d\n", cur_node->left->value);
                fprintf(stderr, "in takeout mul\n");

    fprintf(stderr, "l = %d, r = %d, r->l = %d, r->r = %d\n\n", IsComplex(left_arg), IsComplex(right_arg), IsComplex(right_arg->left), IsComplex(right_arg->right));

    if (!IsComplex(left_arg)        && IsComplex(right_arg) &&
        !IsComplex(right_arg->left) && IsComplex(right_arg->right))
    {
    fprintf(stderr, "in takeout mul gooo\n");
        cur_node->right = right_arg->right;
        cur_node->left  = right_arg;

        right_arg->right = left_arg;
    }

    return cur_node;
}

Node *TakeOutConstsInDiv(Tree *tree, Node *cur_node)                        // (a * x) / b  =>  (a / b) * x 
{
    assert(tree);
    assert(cur_node);

    Node *left_arg  = cur_node->left;
    Node *right_arg = cur_node->right; 

    if (left_arg->type != OP || left_arg->value != DIV)
            return cur_node;

    if (IsComplex(left_arg)       && !IsComplex(right_arg) && 
        !IsComplex(left_arg->left) && IsComplex(left_arg->right))
    {
        cur_node->right = left_arg->right;
        left_arg->right = right_arg;

        left_arg->value = DIV;
        cur_node->value = MUL;
    }

    return cur_node;
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

void VarsToGeneralform(Tree *tree)
{
    assert(tree);

    size_t start_tree_size = tree->size;

    for (size_t i = 0; i < start_tree_size; i++)
    {
        fprintf(stderr, "porno, i = %lld, tree->size = %lld\n", i, tree->size);

        Node *cur_node = tree->node_ptrs[i];

        if (cur_node == NULL)
            continue;

        if (cur_node->type == VAR)
        {
            Node *cur_node_cpy = TreeCopyPaste(tree, tree, cur_node);

            Node *degree     = NewNode(tree, NUM, 1, NULL, NULL);
            Node *multiplier = NewNode(tree, NUM, 1, NULL, NULL);

            Node *deg_node   = NewNode(tree, OP, DEG, cur_node_cpy, degree);

            cur_node->left  = multiplier;
            cur_node->right = deg_node;
            cur_node->type  = OP;
            cur_node->value = MUL;
        }
    }
}

void VarsToNormalForm(Tree *tree)
{
    assert(tree);

    fprintf(LogFile, "before VarsToNormalForm()\n");
    DIFF_DUMP(tree);

    for (size_t i = 0; i < tree->size; i++)
    {
        fprintf(stderr, "i = %lld, tree->size = %lld\n", i, tree->size);

        Node *cur_node = tree->node_ptrs[i];

        if (cur_node == NULL)
            continue;

        if (IsSimpleVarMember(cur_node))
        {
            Node *multiplier = cur_node->left;
            Node *deg_node   = cur_node->right;

            if (deg_node->right->type == NUM && deg_node->right->value == 1)
            {
                fprintf(stderr, "\nGOVNO 11\n");
                Node new_node = *deg_node->left;

                RemoveNode(tree, &deg_node->left);
                RemoveNode(tree, &deg_node->right);

                *deg_node = new_node;
            }

            if (multiplier->type == NUM && multiplier->value == 1)
            {
                fprintf(stderr, "\nGOVNO 22\n");

                Node new_node = *deg_node;

                RemoveNode(tree, &multiplier);
                RemoveNode(tree, &deg_node);

                *cur_node = new_node;
            }
        }
    }

    fprintf(LogFile, "after VarsToNormalForm()\n");
    DIFF_DUMP(tree);
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

            if (arg_2->type == OP && arg_2->value == MUL && arg_2->left->type == NUM && arg_2->left->value < 0)
            {
                cur_node->value = SUB;

                if (arg_2->left->value == -1)   // частный случай
                {       
                    cur_node->right = arg_2->right;

                    RemoveNode(tree, &arg_2->left);
                    RemoveNode(tree, &arg_2);
                }

                else
                    arg_2->left->value *= -1;
            }

            else if (arg_2->type == NUM && arg_2->value < 0)
            {
                cur_node->value = SUB;
                arg_2->value *= -1;   
            }
        }
    }
}

void ExpandAddBrackets(Tree *tree, Node *mul_node)                 // a * (x + y)
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

void ExpandDegBrackets(Tree *tree, Node *deg_op_node)                 // (f * g) ^ a  --> f^a * g^a
{
    fprintf(LogFile, "before ExpandDegBrackets()\n");
    DIFF_DUMP(tree);


    Node *basis  = deg_op_node->left;
    Node *degree = deg_op_node->right;

    Node *res_node  = deg_op_node;
    res_node->value = MUL;

    Node *new_basis_1  = basis->left;
    Node *new_basis_2  = basis->right;
    Node *degree_cpy_1 = TreeCopyPaste(tree, tree, degree);
    Node *degree_cpy_2 = TreeCopyPaste(tree, tree, degree);

    res_node->left->value = DEG;
    res_node->left->right = degree_cpy_1;

    RemoveSubtree(tree, &res_node->right);
    
    res_node->right = NewNode(tree, OP, DEG, NULL, NULL);

    res_node->right->left  = new_basis_2;
    res_node->right->right = degree_cpy_2;

    fprintf(LogFile, "after ExpandDegBrackets()\n");
    DIFF_DUMP(tree);
}


Node *AddFractions(Tree *tree, Node *add_node)      // (a / b) + (c / d)
{
    assert(tree);
    assert(add_node);

    Node *node_a = add_node->left->left;
    Node *node_b = add_node->left->right;
    Node *node_c = add_node->right->left;
    Node *node_d = add_node->right->right;

    Node *node_b_cpy = TreeCopyPaste(tree, tree, node_b);
    Node *node_d_cpy = TreeCopyPaste(tree, tree, node_d);

    Node *new_numerator  = add_node->left;
    new_numerator->value = ADD;

    new_numerator->left  = NewNode(tree, OP, MUL, node_a, node_d_cpy);
    new_numerator->right = NewNode(tree, OP, MUL, node_b_cpy, node_c);

    Node *new_denominator = add_node->right;
    new_denominator->value = MUL;

    new_denominator->left  = node_b;
    new_denominator->right = node_d;

    add_node->value = DIV;

    return add_node;
}

Node *FracPlusNum(Tree *tree, Node *add_node)
{
    assert(tree);
    assert(add_node);

    Node **num  = NULL;

    if (add_node->right->value == NUM)
        num = &add_node->right;

    else
        num = &add_node->left;

    *num = NewNode(tree, OP, DIV, *num, NewNode(tree, NUM, 1, NULL, NULL));

    return AddFractions(tree, add_node);
}

bool IsSimpleFraction(Node *node)
{
    assert(node);
    return (node->type == OP && node->value == DIV && node->left->type == NUM && node->right->type == NUM);
}

// VarMemberInfo GetVarMemberInfo(Node *node)     // 5 * x^(2 / 3)
// {
//     assert(node);

//     VarMemberInfo info = {};

//     if (node->type == VAR)
//     {
//         info.member_type = MEMBER_VAR;
//         info.is_simple_var_member = true;
//         info.degree = 1;
//         info.multiplier = 1;
//     }

//     else if (node->type == OP && node->value == DEG && node->left->type == VAR)
//     {
//         info.is_simple_var_member = true;
//         info.degree = node->right->value;
//         info.multiplier = 1;
//     }

//     else if (node->type == OP && node->value == MUL)
//     {
//         Node *right_mul = node->right;

//         if (right_mul->type == VAR)
//         {
//             info.is_simple_var_member = true;
//             info.degree = 1;
//             info.multiplier = node->left->value;
//         }

//         else if (right_mul->type == OP && right_mul->type == DEG
//               && right_mul->left->type == VAR && !IsComplex(right_mul->right))
//         {
//             info.is_simple_var_member = true;
//             info.degree = right_mul->value;
//         }
//     }
// }

bool IsSimpleVarMember(Node *node)          // example (-4) * x ^(7/9)
{
    if (CHECK_NODE_OP(node, MUL))
    {
        Node *right_multiplier = node->right;

        if (right_multiplier->type == OP && right_multiplier->value == DEG
              && right_multiplier->left->type == VAR && !IsComplex(right_multiplier->right))
        {
            return true;
        }
    }

    return false;
}

bool FractionsAreEqual(Node *div_1, Node *div_2)        // 3/2 != 6/4
{
    assert(div_1);
    assert(div_2);
    assert(CHECK_NODE_OP(div_1, DIV) && CHECK_NODE_OP(div_2, DIV));

    if (IsComplex(div_1) || IsComplex(div_2))
        return false;

    return (div_1->left->value == div_2->left->value && div_1->right->value == div_2->right->value);
}