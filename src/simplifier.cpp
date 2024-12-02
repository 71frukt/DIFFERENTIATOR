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


    SubToAdd(expr_tree);

    VarsToGeneralform(expr_tree);                   // x --> 1 * x ^ 1

    fprintf(LogFile, "before SimplifyConstants()\n");
    DIFF_DUMP(expr_tree);
    SimplifyConstants (expr_tree, expr_tree->root_ptr);

    SimplifyVars      (expr_tree, expr_tree->root_ptr);

    SimplifyConstants (expr_tree, expr_tree->root_ptr);

 fprintf(LogFile, "before VarsToNormalForm()\n");
    DIFF_DUMP(expr_tree);

    VarsToNormalForm(expr_tree);

fprintf(LogFile, "before AddToSub()\n");
    DIFF_DUMP(expr_tree);

    AddToSub(expr_tree);

    DIFF_DUMP(expr_tree);

    fprintf(stderr, "zakonchila yproshalka\n");
}

Node *SimplifyConstants(Tree *tree, Node *cur_node)
{
    assert(cur_node);

    fprintf(stderr, "root_ptr = %p\n", tree->root_ptr);

    const Operation *op  = GetOperationByNode(cur_node);

    if (op != NULL)
        cur_node = op->simpl_nums_func(tree, cur_node);

    return cur_node;
}

Node *SimplifyVars(Tree *tree, Node *cur_node)
{
    assert(cur_node);

    fprintf(LogFile, "in SimplifyVars(): cur_node->val = %d\n", cur_node->val.num);

    const Operation *op  = GetOperationByNode(cur_node);

    if (op != NULL)
    {
        fprintf(stderr, "op val = %d\n", op->num);
        cur_node = op->simpl_vars_func(tree, cur_node);
    }

    return cur_node;
}

Node *SimplNumsAdd(Tree *tree, Node *add_node)
{
    SimplNumsArgs(tree, add_node);

    if (add_node->left->type == NUM && add_node->right->type == NUM)
    {
        CalculateOp(tree, add_node);
        return add_node;
    }

    else if (add_node->left->type == NUM && add_node->left->val.num == 0)
    {
        RemoveNode(tree, &add_node->left);
        // RemoveNode(tree, &add_node);

        Node *right = add_node->right;

        *add_node = *right;
        RemoveNode(tree, &right);

        return SimplifyConstants(tree, add_node);
    }

    else if (IsSimpleFraction(add_node->left) && IsSimpleFraction(add_node->right))
    {
        Node *new_fract_node = AddFractions(tree, add_node);               // ADD --> DIV
        SimplifyConstants(tree, new_fract_node);
        return new_fract_node;
    }

    else if ((IsSimpleFraction(add_node->left)  && add_node->right->type == NUM)
     || (IsSimpleFraction(add_node->right) && add_node->left->type  == NUM))
    {
        Node *new_fract_node = FracPlusNum(tree, add_node);               // ADD --> DIV

        SimplifyConstants(tree, new_fract_node);


        return new_fract_node;
    }

    else
    {
        ComplexToTheRight(add_node);

        TakeOutConsts(tree, add_node);
        SimplifyConstants(tree, add_node->left);

        return add_node;
    }
}

Node *SimplNumsSub(Tree *tree, Node *sub_node)
{
    SimplNumsArgs(tree, sub_node);

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
    SimplNumsArgs(tree, mul_node);

    fprintf(stderr, "SimplNumsMul()\n");
    // fprintf(LogFile, "SimplNumsMul()\n");
    // DIFF_DUMP(tree);


    if (mul_node->left->type == NUM && mul_node->right->type == NUM)
    {
        CalculateOp(tree, mul_node);
        return mul_node;
    }

    ComplexToTheRight(mul_node);
    TakeOutConsts(tree, mul_node);

    if (mul_node->left->type == NUM && mul_node->left->val.num == 0)
    {
        return MulByNull(tree, mul_node);
    }

    if (MulByFraction(mul_node, mul_node->left, mul_node->right))                           // если есть умножение на дробь  MUL --> DIV
    {
        return SimplifyConstants(tree, mul_node);
    }

    bool is_still_mul = (mul_node->type == OP && mul_node->val.op == MUL);   // если не свернулось в константу

    if (is_still_mul)
    {
        bool left_is_simple_multiplier = !SubtreeContComplicOperation(mul_node->left);
        bool right_is_sum = (mul_node->right->type == OP && mul_node->right->val.op == ADD);

        if (left_is_simple_multiplier && right_is_sum)      // раскрыть скобки
        {
            fprintf(stderr, "ExpandAddBrackets\n");

            ExpandAddBrackets(tree, mul_node);                 // MUL -> ADD
            SimplifyConstants(tree, mul_node);
            return mul_node;
        }
    }


    return mul_node;
}

Node *SimplNumsDiv(Tree *tree, Node *div_node)
{
    SimplNumsArgs(tree, div_node);

    if (TakeOutConstsInDiv(tree, div_node)->val.op != DIV)    // (a * x) / b  =>  (a / b) * x  --- упростилось и sub_node->type != DIV
    {
        SimplifyConstants(tree, div_node->left);
        return div_node;
    }

    if (div_node->left->type == NUM && div_node->left->val.op == 0)
        return FractionBecomesZero(tree, div_node);

    Node *numerator   = div_node->left;
    Node *denominator = div_node->right;

    bool numerator_is_int   = IS_INT_VAL(div_node->left);
    bool denominator_is_int = IS_INT_VAL(div_node->right);

    bool can_calc = ((!numerator_is_int || !denominator_is_int) && (numerator->type == NUM && denominator->type == NUM)) ||         // оба double
                     ((numerator_is_int &&  denominator_is_int) && (numerator->val.num % denominator->val.num) == 0);                       // или делятся нацело

    if (numerator->type == NUM && denominator->type == NUM && can_calc)
    {
        CalculateOp(tree, div_node);
        return div_node;
    }

    else if ((numerator->type   == OP && numerator->val.op   == DIV)
          || (denominator->type == OP && denominator->val.op == DIV))
    {
        FlipDenominator(div_node);
        SimplifyConstants(tree, div_node);
    }

    else if (numerator_is_int && denominator_is_int)   // и числитель, и знаменатель целые
    {
        TreeElem_t start_val = MIN(abs(numerator->val.num), abs(denominator->val.num));

        for (TreeElem_t divider = start_val; divider > 0; divider--)
        {
            if (divider > abs(numerator->val.num))
                divider = abs(numerator->val.num);


            if (numerator->val.num % divider == 0 && denominator->val.num % divider == 0)
            {
                numerator->val.num   /= divider;
                denominator->val.num /= divider;
            }
        }
    }

    if (denominator->type == NUM && denominator->val.num < 0)
    {
        denominator->val.num *= -1;

        Node *minus    = NewNode(tree, NUM, {.num = -1},  NULL, NULL);
        div_node->left = NewNode(tree, OP,  {.op  = MUL}, minus, numerator);

        SimplifyConstants(tree, div_node->left);
    }

    return div_node;    
}

Node *SimplNumsDeg(Tree *tree, Node *deg_node)
{
    assert(tree);
    assert(deg_node);

    SimplNumsArgs(tree, deg_node);

    Node *basis  = deg_node->left;
    Node *degree = deg_node->right;

    bool basis_is_valid  = ((IS_INT_TREE && basis->type  == NUM)                        || !IS_INT_TREE);
    bool degree_is_valid = ((IS_INT_TREE && degree->type == NUM && degree->val.num > 0) || !IS_INT_TREE);

    if (basis->type == NUM && basis->val.num == 1)
    {
        Node one = *basis;

        RemoveSubtree(tree, &basis);
        RemoveSubtree(tree, &degree);

        *deg_node = one;
        return deg_node;
    }

    else if (basis_is_valid && degree_is_valid)
    {
        CalculateOp(tree, deg_node);
        return deg_node;
    }

    else if (basis->type == OP && basis->val.op == DEG)
    {
        RevealDoubleDeg(deg_node);
        SimplifyConstants(tree, deg_node);
        return deg_node;
    }

    else if (basis->type == OP && basis->val.op == MUL && !IsComplex(degree))
    {
        ExpandDegBrackets(tree, deg_node);      // DEG --> MUL

        SimplifyConstants(tree, deg_node);
        return deg_node;
    }

    else if (IsSimpleFraction(basis) && !IsComplex(basis))
    {
        FractionInDeg(tree, deg_node);              // DEG --> DIV

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

    Node *new_basis    = old_basis->left;
    Node *new_degree   = old_basis;
    new_degree->val.op = MUL;

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

    SimplVarsArgs(tree, sub_node);

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

        bool degrees_are_equal = ((deg_1->type == NUM && deg_2->type == NUM && deg_1->val.num == deg_2->val.num) 
                               || (CHECK_NODE_OP(deg_1, DIV) && CHECK_NODE_OP(deg_2, DIV) && FractionsAreEqual(deg_1, deg_2)));

        if (var_1->val.var == var_2->val.var && degrees_are_equal)
        {
            TreeElem_t new_mult_val = mult_1->val.num + mult_2->val.num;

            Node *new_mult    = mult_1;
            new_mult->val.num = new_mult_val;

            Node *res_node   = sub_node;
            res_node->val.op = MUL;

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

        // else if (var_1->val == var_2->val && CHECK_NODE_OP(deg_1, DIV) && CHECK_NODE_OP(deg_2, DIV) && FractionsAreEqual(deg_1, deg_2))
        // {

        // }
    }

    return sub_node;
}


Node *SimplVarsMul(Tree *tree, Node *mul_node)
{
    fprintf(stderr, "SIMPL MUL: VAR!!!!\n\n");

    assert(tree);
    assert(mul_node);

    // SimplVarsArgs(tree, sub_node);

    Node *arg_1 = mul_node->left;
    Node *arg_2 = mul_node->right;

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
            return mul_node;
        }

        RemoveNode(tree, &var_1);
        RemoveNode(tree, &arg_1->right);
        arg_1->right = mult_2;

        arg_2->val.op = DEG;
        arg_2->left   = var_2;

        arg_2->right->val.op = ADD;
        arg_2->right->left   = deg_1;
    }

    fprintf(stderr, "done\n");

    return mul_node;
}

Node *SimplNumsArgs(Tree *tree, Node *op_node)
{
    assert(op_node);
    assert(op_node->left);
    assert(op_node->right);

    const Operation *this_op  = GetOperationByNode(op_node);

    if (this_op == NULL)
        return op_node;

    const Operation *left_op  = GetOperationByNode(op_node->left);

    if (left_op != NULL)
        op_node->left = left_op->simpl_nums_func(tree, op_node->left);

    
    if (this_op->type == BINARY)
    {
        const Operation *right_op = GetOperationByNode(op_node->right);   

        if (right_op != NULL)
            op_node->right = right_op->simpl_nums_func(tree, op_node->right);
    }

    return op_node;
}

Node *SimplVarsArgs(Tree *tree, Node *op_node)
{
    assert(op_node);
    assert(op_node->left);
    assert(op_node->right);

    const Operation *this_op  = GetOperationByNode(op_node);

    if (this_op == NULL)
        return op_node;

    const Operation *left_op  = GetOperationByNode(op_node->left);

    if (left_op != NULL)
        op_node->left = left_op->simpl_vars_func(tree, op_node->left);

    
    if (this_op->type == BINARY)
    {
        const Operation *right_op = GetOperationByNode(op_node->right);   

        if (right_op != NULL)
            op_node->right = right_op->simpl_vars_func(tree, op_node->right);
    }

    return op_node;
}

TreeElem_t CalculateOp(Tree *tree, Node *op_node)
{
    assert(tree);
    assert(op_node);

    const Operation *op = GetOperationByNode(op_node);
    assert(op);

    TreeElem_t new_val = op->op_func(op_node->left->val.num, op_node->right->val.num);

    RemoveNode(tree, &op_node->left);
    RemoveNode(tree, &op_node->right);
    op_node->left  = NULL;
    op_node->right = NULL;

    op_node->type    = NUM;
    op_node->val.num = new_val;

    return new_val;
}

Node *SimplifyFraction(Tree *tree, Node *op_node, Node *numerator, Node *denominator)
{
    assert(tree);
    assert(numerator);
    assert(denominator);

    if (numerator->type == NUM && numerator->val.num == 0)
        return FractionBecomesZero(tree, op_node);

    bool numerator_is_int   = IS_INT_VAL(numerator);
    bool denominator_is_int = IS_INT_VAL(denominator);

    bool can_calc = ((!numerator_is_int || !denominator_is_int) && (numerator->type == NUM && denominator->type == NUM)) ||          // они double
                     ((numerator_is_int &&  denominator_is_int) && (numerator->val.num % denominator->val.num) == 0);                // или делятся нацело

    if (numerator->type == NUM && denominator->type == NUM && can_calc)
    {
        TreeElem_t new_val = numerator->val.num / denominator->val.num;
        
        RemoveNode(tree, &op_node->left);
        RemoveNode(tree, &op_node->right);

        op_node->left  = NULL;
        op_node->right = NULL;

        op_node->type    = NUM;
        op_node->val.num = new_val;
    }

    else if ((numerator->type   == OP && numerator->val.op   == DIV)
          || (denominator->type == OP && denominator->val.op == DIV))
    {
        FlipDenominator(op_node);
        SimplifyConstants(tree, op_node);
    }

    else if (numerator_is_int && denominator_is_int)   // и числитель, и знаменатель целые
    {
        TreeElem_t start_val = MIN(abs(numerator->val.num), abs(denominator->val.num)) / 2;

        for (TreeElem_t divider = start_val; divider > 0; divider--)
        {
            if (divider > numerator->val.num)
                divider = numerator->val.num;
            
            if (numerator->val.num % divider == 0 && denominator->val.num % divider == 0)
            {
                numerator->val.num   /= divider;
                denominator->val.num /= divider;
            }
        }
    }

    else if (denominator->type == NUM && denominator->val.num < 0)
    {
        denominator->val.num *= -1;

        Node *minus   = NewNode(tree, NUM, {.num = -1}, NULL, NULL);
        op_node->left = NewNode(tree, OP,  {.op = MUL}, minus, numerator);

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

    if (denom->type == OP && denom->val.op == DIV)   // в знаменателе дробь
    {
        fprintf(stderr, "WATAFAK\n\n");

        Node *numerator_of_denom = denom->left;

        denom->val.op = MUL;
        denom->left   = fraction_node->left;

        fraction_node->left  = denom;
        fraction_node->right = numerator_of_denom;
    }

    else                                            // в знаменателе не дробь => в числителе дробь
    {
        fprintf(stderr, "HUETAAA\n");

        Node *prev_denom = fraction_node->right;

        fraction_node->right = fraction_node->left;
        fraction_node->right->val.op = MUL;

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

    bool left_is_fraction  = (left_arg->type  == OP && left_arg->val.op  == DIV);
    bool right_is_fraction = (right_arg->type == OP && right_arg->val.op == DIV);

    bool left_is_complex  = IsComplex(left_arg);
    bool right_is_complex = IsComplex(right_arg);

    if (left_is_fraction && !right_is_fraction && !right_is_complex)
    {
        // mul_node->val = DIV;
        mul_node->left->val.op = MUL;

        mul_node->right = left_arg->right;
        left_arg->right = right_arg;
    }

    else if (right_is_fraction && !left_is_fraction && !left_is_complex)
    {
        // mul_node->val = DIV;

        mul_node->left = right_arg;
        mul_node->left->val.op = MUL;

        mul_node->right = right_arg->right;
        right_arg->right = left_arg;
    }

    else if (left_is_fraction && right_is_fraction)
    {
        left_arg->val.op  = MUL;
        right_arg->val.op = MUL;

        Node *tmp_node = left_arg->right;

        left_arg->right = right_arg->left;
        right_arg->left = tmp_node;
    }

    else
        return false;

    mul_node->val.op = DIV;

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

Node *TakeOutConsts(Tree *tree, Node *cur_node)                        //      a + (b + f(x))    =>  (a + b) + f(x)            (1)
{                                                                      // или (a + f(x)) + g(x)  =>  a + (f(x) + g(x))         (2)
    assert(tree);
    assert(cur_node);

    Node *left_arg  = cur_node->left;
    Node *right_arg = cur_node->right; 
    
    bool have_equal_types_1 = (CHECK_NODE_OP(cur_node, ADD) && CHECK_NODE_OP(right_arg, ADD))
                           || (CHECK_NODE_OP(cur_node, MUL) && CHECK_NODE_OP(right_arg, MUL));

    bool have_equal_types_2 = (CHECK_NODE_OP(cur_node, ADD) && CHECK_NODE_OP(left_arg, ADD))
                           || (CHECK_NODE_OP(cur_node, MUL) && CHECK_NODE_OP(left_arg, MUL));

    if (have_equal_types_1           &&                                         // раскрытие (1)
       !IsComplex(left_arg)          && IsComplex(right_arg) &&
       !IsComplex(right_arg->left)   && IsComplex(right_arg->right))
    {
        fprintf(stderr, "\n\n\nTAKEOUTCONSTS   (1)\n\n");
        cur_node->right = right_arg->right;
        cur_node->left  = right_arg;

        right_arg->right = left_arg;

        SimplifyConstants(tree, cur_node->left);
    }

    else if (have_equal_types_2 && !IsComplex(left_arg->left) && !IsSimpleVarMember(left_arg))
    {
        fprintf(stderr, "\n\n\nTAKEOUTCONSTS    (2)\n\n");

        cur_node->right = left_arg->left;
        left_arg->left  = right_arg;
        ComplexToTheRight(cur_node);

        SimplifyConstants(tree, cur_node->right);
    }

    return cur_node;
}

// Node *TakeOutConstsInMul(Tree *tree, Node *cur_node)                        //     a * (b * x)  =>  (a * b) * x
// {                                                                           // или (a * f(x)) * g(x) => a * (f(x) * g(x))
//     assert(tree);
//     assert(cur_node);

//     Node *left_arg  = cur_node->left;
//     Node *right_arg = cur_node->right;

//     fprintf(stderr, "\n\n\ncur_node->left = %d\n", cur_node->left->val);
//                 fprintf(stderr, "in takeout mul\n");

//     fprintf(stderr, "l = %d, r = %d, r->l = %d, r->r = %d\n\n", IsComplex(left_arg), IsComplex(right_arg), IsComplex(right_arg->left), IsComplex(right_arg->right));

//     if (CHECK_NODE_OP(right_arg, MUL) &&
//         !IsComplex(left_arg)        && IsComplex(right_arg) &&
//         !IsComplex(right_arg->left) && IsComplex(right_arg->right))
//     {
//     fprintf(stderr, "in takeout mul gooo\n");
//         cur_node->right = right_arg->right;
//         cur_node->left  = right_arg;

//         right_arg->right = left_arg;
//     }

//     return cur_node;
// }

Node *TakeOutConstsInDiv(Tree *tree, Node *cur_node)                        // (a * x) / b  =>  (a / b) * x 
{
    assert(tree);
    assert(cur_node);

    Node *left_arg  = cur_node->left;
    Node *right_arg = cur_node->right; 

    if (left_arg->type != OP || left_arg->val.op != MUL)
            return cur_node;

    if (IsComplex(left_arg)       && !IsComplex(right_arg) && 
        !IsComplex(left_arg->left) && IsComplex(left_arg->right))       // TODO !Exists vars instead
    {
        cur_node->right = left_arg->right;
        left_arg->right = right_arg;

        left_arg->val.op = DIV;
        cur_node->val.op = MUL;
    }

    return cur_node;
}

Node *FractionBecomesZero(Tree *tree, Node *div_node)
{
    Node *denominator = div_node->right;

    if (denominator->type == NUM && denominator->val.num == 0)
    {
        fprintf(stderr, "%p\n", div_node);

        fprintf(stderr, "Warning: uncertainty type of '0/0'\n");
        return div_node;
    }

    RemoveNode(tree, &div_node->left);
    RemoveNode(tree, &div_node->right);

    div_node->left  = NULL;
    div_node->right = NULL;

    div_node->type    = NUM;
    div_node->val.num = 0;

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

        if (cur_node->type == OP && cur_node->val.op == SUB)
        {
            Node *minus = NewNode(tree, NUM, {.num = -1}, NULL, NULL);
            cur_node->right = NewNode(tree, OP, {.op = MUL}, minus, cur_node->right);
            cur_node->val.op = ADD;
        }
    }
}

void VarsToGeneralform(Tree *tree)
{
    assert(tree);

    size_t start_tree_size = tree->size;

    for (size_t i = 0; i < start_tree_size; i++)
    {
        Node *cur_node = tree->node_ptrs[i];

        if (cur_node == NULL)
            continue;

        if (cur_node->type == VAR)
        {
            Node *cur_node_cpy = TreeCopyPaste(tree, tree, cur_node);

            Node *degree     = NewNode(tree, NUM, {.num = 1}, NULL, NULL);
            Node *multiplier = NewNode(tree, NUM, {.num = 1}, NULL, NULL);

            Node *deg_node   = NewNode(tree, OP, {.op = DEG}, cur_node_cpy, degree);

            cur_node->left   = multiplier;
            cur_node->right  = deg_node;
            cur_node->type   = OP;
            cur_node->val.op = MUL;
        }
    }
}

void VarsToNormalForm(Tree *tree)
{
    assert(tree);

    for (size_t i = 0; i < tree->size; i++)
    {
        Node *cur_node = tree->node_ptrs[i];

        if (cur_node == NULL || cur_node->type != OP || !CHECK_NODE_OP(cur_node, MUL))
            continue;

        Node *multiplier = cur_node->left;
        Node *deg_node   = cur_node->right;

        if (IsSimpleVarMember(cur_node) && deg_node->right->type == NUM && deg_node->right->val.num == 1)
        {
            fprintf(stderr, "\nGOVNO 11\n");
            Node new_node = *deg_node->left;

            RemoveNode(tree, &deg_node->left);
            RemoveNode(tree, &deg_node->right);

            *deg_node = new_node;
        }
        
        if (multiplier->type == NUM && multiplier->val.num == 1)
        {
            fprintf(stderr, "\nGOVNO 22\n");

            RemoveNode(tree, &multiplier);
            // RemoveNode(tree, &add_node);

            Node *right = cur_node->right;

            *cur_node = *right;
            RemoveNode(tree, &right);
        }

        // if (mul_node->left->type == NUM && mul_node->left->val == 1)
        // {
        //     RemoveNode(tree, &mul_node->left);
        //     // RemoveNode(tree, &add_node);

        //     Node *right = mul_node->right;

        //     *mul_node = *right;
        //     RemoveNode(tree, &right);
        // }
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
        
        if (cur_node->type == OP && cur_node->val.op == ADD)
        {
            Node *arg_2 = cur_node->right;

            if (arg_2->type == OP && arg_2->val.op == MUL && arg_2->left->type == NUM && arg_2->left->val.num < 0)
            {
                cur_node->val.op = SUB;

                if (arg_2->left->val.num == -1)   // частный случай
                {       
                    cur_node->right = arg_2->right;

                    RemoveNode(tree, &arg_2->left);
                    RemoveNode(tree, &arg_2);
                }

                else
                    arg_2->left->val.num *= -1;
            }

            else if (arg_2->type == NUM && arg_2->val.num < 0)
            {
                cur_node->val.op = SUB;
                arg_2->val.num  *= -1;   
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

    Node *left_mul = NewNode(tree, OP, {.op = MUL}, simple_multiplier, mul_node->right->left);

    Node *right_mul = mul_node->right;
    right_mul->val.op = MUL;

    right_mul->left = simple_multiplier_cpy;
    // right_mul->right остаётся тем же, что и был до этого

    mul_node->val.op = ADD;
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

    Node *basis  = deg_op_node->left;
    Node *degree = deg_op_node->right;

    Node *res_node   = deg_op_node;
    res_node->val.op = MUL;

    // Node *new_basis_1  = basis->left;
    Node *new_basis_2  = basis->right;
    Node *degree_cpy_1 = TreeCopyPaste(tree, tree, degree);
    Node *degree_cpy_2 = TreeCopyPaste(tree, tree, degree);

    res_node->left->val.op = DEG;
    res_node->left->right  = degree_cpy_1;

    RemoveSubtree(tree, &res_node->right);
    
    res_node->right = NewNode(tree, OP, {.op = DEG}, NULL, NULL);

    res_node->right->left  = new_basis_2;
    res_node->right->right = degree_cpy_2;

    fprintf(LogFile, "after ExpandDegBrackets()\n");
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

    Node *new_numerator   = add_node->left;
    new_numerator->val.op = ADD;

    new_numerator->left  = NewNode(tree, OP, {.op = MUL}, node_a, node_d_cpy);
    new_numerator->right = NewNode(tree, OP, {.op = MUL}, node_b_cpy, node_c);

    Node *new_denominator   = add_node->right;
    new_denominator->val.op = MUL;

    new_denominator->left  = node_b;
    new_denominator->right = node_d;

    add_node->val.op = DIV;

    return add_node;
}

Node *FracPlusNum(Tree *tree, Node *add_node)
{
    assert(tree);
    assert(add_node);

    Node *num  = NULL;

    if (add_node->right->type == NUM)
    {
        num = add_node->right;
        Node *fract_2 = NewNode(tree, OP, {.op = DIV}, num, NewNode(tree, NUM, {.num = 1}, NULL, NULL));
        add_node->right = fract_2;
    }

    else
    {
        num = add_node->left;
        Node *fract_2 = NewNode(tree, OP, {.op = DIV}, num, NewNode(tree, NUM, {.num = 1}, NULL, NULL));
        add_node->left = fract_2;
    }

    return AddFractions(tree, add_node);
}

Node *FractionInDeg(Tree *tree, Node *pow_node)
{
    assert(tree);
    assert(pow_node);

    Node *degree = pow_node->right;
    Node *degree_cpy = TreeCopyPaste(tree, tree, degree);
    Node *fraction = pow_node->left;

    assert(IsSimpleFraction(pow_node->left));
    assert(!IsComplex(degree));

    // Node *numerator   = fraction->left;
    Node *denominator = fraction->right;

    pow_node->val.op = DIV;
    fraction->val.op = DEG;
    fraction->right = degree_cpy;
    
    pow_node->right = NewNode(tree, OP, {.op = DEG}, denominator, degree);

    return pow_node;
}

Node *MulByNull(Tree *tree, Node *mul_node)
{
    assert(tree);
    assert(mul_node);

    Node null_node = *mul_node->left;

    RemoveNode   (tree, &mul_node->left);
    RemoveSubtree(tree, &mul_node->right);

    *mul_node = null_node;

    return mul_node;
}

bool IsSimpleFraction(Node *node)
{
    assert(node);
    return (node->type == OP && node->val.op == DIV && node->left->type == NUM && node->right->type == NUM);
}

bool IsSimpleVarMember(Node *node)          // example (-4) * x ^(7/9)
{
    if (CHECK_NODE_OP(node, MUL))
    {
        Node *right_multiplier = node->right;

        if (right_multiplier->type == OP && right_multiplier->val.op == DEG
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

    return (div_1->left->val.num == div_2->left->val.num && div_1->right->val.num == div_2->right->val.num);
}