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

fprintf(LogFile, "start of TakeDerivative():\n");

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

        char tex_cur_change_name[TEX_CHANGE_NAME_LEN] = {};
        GetTexChangedVarName(cur_change, tex_cur_change_name);
        fprintf(OutputFile, "Lets take a derivative of %s\\newline\n", tex_cur_change_name);
        
        int     diff_change_deriv_num = cur_change->derivative_num + 1;
        int     diff_change_name      = cur_change->name;
        Change *diff_change = &(diff_tree->changed_vars.data[diff_tree->changed_vars.size++]);
        diff_change->name           = (char) diff_change_name;
        diff_change->derivative_num = diff_change_deriv_num;
        
        diff_change->target_node = TakeDerivative(expr_tree, cur_change->target_node, diff_tree);

        char tex_diff[TEX_EXPRESSION_LEN] = {};
        GetTexTreeData(diff_change->target_node, tex_diff, false);

        char tex_change_name[TEX_CHANGE_NAME_LEN] = {};
        GetTexChangedVarName(diff_change, tex_change_name);
        SimplifyExpr(diff_tree, diff_change->target_node);
        Node *new_node = NewNode(diff_tree, CHANGE, {.change = diff_change}, NULL, NULL);

    // fprintf(LogFile, "after make new A(2)\n");
    // DIFF_DUMP(diff_tree);

        char res_change_name_tex[TEX_CHANGE_NAME_LEN] = {};
        GetTexChangedVarName(diff_change, res_change_name_tex);

        char res_diff_tex[TEX_EXPRESSION_LEN] = {};
        GetTexTreeData(diff_change->target_node, res_diff_tex, false);

        fprintf(OutputFile, "So $%s = %s$\\newline\n\\newline\n", res_change_name_tex, res_diff_tex);
fprintf(stderr, "So $%s = %s$\\newline\n\\newline\n", res_change_name_tex, res_diff_tex);

        res_node = new_node;
    }

    else if (expr_node->type == OP)
    {
        char differentiated_tex[TEX_EXPRESSION_LEN] = {};
        GetTexTreeData(expr_node, differentiated_tex, false);
        fprintf(OutputFile, "Trying to take a derivative of $%s$...\\newline\n\\newline\n", differentiated_tex);

        const Operation *cur_op = GetOperationByNode(expr_node);

        res_node = cur_op->diff_func(expr_tree, expr_node, diff_tree);
        
        SimplifyExpr(diff_tree, res_node);
        char res_diff_tex[TEX_EXPRESSION_LEN] = {};
        GetTexTreeData(res_node, res_diff_tex, false);

        fprintf(OutputFile, "Having counted the most obvious derivative, which the Soviet spermatozoa were actually able to calculate in their minds, we get:\n$(%s)'(x) = %s$\\newline\n\\newline\n", differentiated_tex, res_diff_tex);
    }

    return res_node;
}

// Node *CalculateTailor(Tree *expr_tree, Node *start_node, Tree *tailor_tree, size_t order)
// {
//     assert(expr_tree);
//     assert(start_node);
//     assert(tailor_tree);

    
// }