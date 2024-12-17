#include <stdio.h>
#include <assert.h>

#include "operations.h"
#include "derivative.h"
#include "diff_debug.h"
#include "tex_work.h"

extern FILE *LogFile;
extern FILE *OutputFile;

Node *TakeHighDerivative(Tree *expr_tree, Node *start_node, Tree *diff_tree, size_t order)
{
    assert(expr_tree);
    assert(start_node);
    assert(diff_tree);

    Tree tmp_diff_tree = {};
    TreeCtor(&tmp_diff_tree, START_TREE_SIZE ON_DIFF_DEBUG(, "tmp_diff_tree"));

    diff_tree->root_ptr = TreeCopyPaste(expr_tree, diff_tree, start_node);

    for (size_t i = 0; i < order; i++)
    {
        tmp_diff_tree.root_ptr = TreeCopyPaste(diff_tree, &tmp_diff_tree, diff_tree->root_ptr);

        RemoveSubtree(diff_tree, &diff_tree->root_ptr);
        diff_tree->size = 0;

        diff_tree->root_ptr = TakeDerivative(&tmp_diff_tree, tmp_diff_tree.root_ptr, diff_tree);
        SimplifyExpr(diff_tree, diff_tree->root_ptr);

        RemoveSubtree(&tmp_diff_tree, &tmp_diff_tree.root_ptr);
        tmp_diff_tree.size = 0;
    }

    TreeDtor(&tmp_diff_tree);

    return diff_tree->root_ptr;
}

Node *TakeDerivative(Tree *expr_tree, Node *expr_node, Tree *diff_tree)
{
    assert(expr_tree);
    assert(expr_node);
    assert(diff_tree);

    if (expr_node->type == NUM)
    {
        return NewNode(diff_tree, NUM, {.num = 0}, NULL, NULL);
    }

    else if (expr_node->type == VAR)
    {
        return NewNode(diff_tree, NUM, {.num = 1}, NULL, NULL);
    }


    Node *res_node = NULL;

    if (expr_node->type == CHANGE)
    {
        Change *cur_change = expr_node->val.change;

        if (cur_change->target_node->type == NUM)
        {
            return NewNode(diff_tree, NUM, {.num = 0}, NULL, NULL);
        }

        char tex_cur_change_name[TEX_CHANGE_NAME_LEN] = {};
        GetTexChangedVarName(cur_change, tex_cur_change_name);
        fprintf(OutputFile, "Lets take a derivative of %s\\newline\n", tex_cur_change_name);
        
        int     diff_change_deriv_num = cur_change->derivative_num + 1;
        int     diff_change_name      = cur_change->name;
        Change *diff_change = &(diff_tree->changed_vars.data[diff_tree->changed_vars.size++]);
        diff_change->name           = (char) diff_change_name;
        diff_change->derivative_num = diff_change_deriv_num;
        
        diff_change->target_node = TakeDerivative(expr_tree, cur_change->target_node, diff_tree);

        char *tex_diff = (char *) calloc (1, STR_EXPRESSION_LEN);
        GetStrTreeData(diff_change->target_node, tex_diff, false, TEX);

        char tex_change_name[TEX_CHANGE_NAME_LEN] = {};
        GetTexChangedVarName(diff_change, tex_change_name);
        SimplifyExpr(diff_tree, diff_change->target_node);
        Node *new_node = NewNode(diff_tree, CHANGE, {.change = diff_change}, NULL, NULL);

        char res_change_name_tex[TEX_CHANGE_NAME_LEN] = {};
        GetTexChangedVarName(diff_change, res_change_name_tex);

        char *res_diff_tex = (char *) calloc (1, STR_EXPRESSION_LEN);
        GetStrTreeData(diff_change->target_node, res_diff_tex, false, TEX);

        fprintf(OutputFile, "So \\[%s = %s\\]\\newline\n\\newline\n", res_change_name_tex, res_diff_tex);

        res_node = new_node;

        free(tex_diff);
        free(res_diff_tex);
    }

    else if (expr_node->type == OP)
    {
        char *differentiated_tex = (char *) calloc (1, STR_EXPRESSION_LEN);

        GetStrTreeData(expr_node, differentiated_tex, false, TEX);
        fprintf(OutputFile, "Trying to take a derivative of $%s$...\\newline\n\\newline\n", differentiated_tex);

        const Operation *cur_op = GetOperationByNode(expr_node);

        res_node = cur_op->diff_func(expr_tree, expr_node, diff_tree);
        
        SimplifyExpr(diff_tree, res_node);

        char *res_diff_tex = (char *) calloc (1, STR_EXPRESSION_LEN);
        GetStrTreeData(res_node, res_diff_tex, false, TEX);

        fprintf(OutputFile, "Having counted the most obvious derivative, which the Soviet spermatozoa were actually able to calculate in their minds, we get:\n$(%s)'(x) = %s$\\newline\n\\newline\n", differentiated_tex, res_diff_tex);

        free(differentiated_tex);
        free(res_diff_tex);
    }

    return res_node;
}

Node *CalculateTailor(Tree *expr_tree, Node *start_node, Tree *tailor_tree, size_t order)
{
    assert(expr_tree);
    assert(start_node);
    assert(tailor_tree);

    Tree tmp_diff_tree = {};
    Tree tmp_res_of_diff = {};
    TreeCtor(&tmp_diff_tree,   START_TREE_SIZE ON_DIFF_DEBUG(, "tmp_diff_tree_tailor"));
    TreeCtor(&tmp_res_of_diff, START_TREE_SIZE ON_DIFF_DEBUG(, "tmp_res_of_diff_tailor"));

    tmp_diff_tree.root_ptr   = TreeCopyPaste(expr_tree, &tmp_diff_tree,   expr_tree->root_ptr);
    tmp_res_of_diff.root_ptr = TreeCopyPaste(expr_tree, &tmp_res_of_diff, expr_tree->root_ptr);

    PutNumInsteadVar(&tmp_res_of_diff, 0);
    SimplifyConstants(&tmp_res_of_diff, tmp_res_of_diff.root_ptr);
    assert(tmp_res_of_diff.root_ptr->type == NUM);

    TreeElem_t res_coeff = tmp_res_of_diff.root_ptr->val.num;

    tailor_tree->root_ptr = NewNode(tailor_tree, NUM, {.num = res_coeff}, NULL, NULL);
    RemoveSubtree(&tmp_res_of_diff,   &tmp_res_of_diff.root_ptr);

    for (size_t i = 1; i <= order; i++)
    {
        fprintf(LogFile, "before der\n");

        tmp_res_of_diff.root_ptr = TakeDerivative(&tmp_diff_tree, tmp_diff_tree.root_ptr, &tmp_res_of_diff);
    fprintf(LogFile, "after der\n");

        SimplifyExpr(&tmp_res_of_diff, tmp_res_of_diff.root_ptr);

        RemoveSubtree(&tmp_diff_tree,   &tmp_diff_tree.root_ptr);
        tmp_diff_tree.root_ptr = TreeCopyPaste(&tmp_res_of_diff, &tmp_diff_tree, tmp_res_of_diff.root_ptr);          // скопировать результат

        PutNumInsteadVar(&tmp_res_of_diff, 0);
        SimplifyConstants(&tmp_res_of_diff, tmp_res_of_diff.root_ptr);
        assert(tmp_res_of_diff.root_ptr->type == NUM);

        TreeElem_t high_deriv_coeff = tmp_res_of_diff.root_ptr->val.num;
        res_coeff = high_deriv_coeff / Factorial((TreeElem_t) i);
        Node *res_coeff_node = NewNode(tailor_tree, NUM, {.num = res_coeff}, NULL, NULL);

        Node *new_add = NewNode(tailor_tree, OP, {.op = ADD}, tailor_tree->root_ptr, NULL);
        tailor_tree->root_ptr = new_add;

        Node *deg_node = NewNode(tailor_tree, OP, {.op = DEG},  NewNode(tailor_tree, VAR, {.var = 'x'}, NULL, NULL),
                                                                NewNode(tailor_tree, NUM, {.num = (TreeElem_t) i},  NULL, NULL));

        Node *new_member = NewNode(tailor_tree, OP, {.op = MUL}, res_coeff_node, deg_node);

        new_add->right = new_member;

        RemoveSubtree(&tmp_res_of_diff, &tmp_res_of_diff.root_ptr);
    }

    SimplifyExpr(tailor_tree, tailor_tree->root_ptr);

    TreeDtor(&tmp_diff_tree);
    TreeDtor(&tmp_res_of_diff);

    return tailor_tree->root_ptr;
}

void PutNumInsteadVar(Tree *tree, TreeElem_t num_val)
{
    assert(tree);

    for (size_t i = 0; i < tree->size; i++)
    {
        Node *cur_node = tree->node_ptrs[i];

        if (cur_node == NULL)
            continue;

        if (cur_node->type == VAR)
        {
            cur_node->type    = NUM;
            cur_node->val.num = num_val;
        }
    }
}

TreeElem_t Factorial (TreeElem_t n) 
{
    return (n < 2) ? 1 : n * Factorial(n - 1);
}