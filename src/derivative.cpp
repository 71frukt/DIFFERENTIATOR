#include <stdio.h>
#include <assert.h>

#include "operations.h"
#include "derivative.h"
#include "diff_debug.h"
#include "tex_work.h"

extern FILE *LogFile;
extern FILE *OutputFile;

Node *TakeDifferential(Tree *expr_tree, Node *expr_node, Tree *solv_tree)
{
    assert(expr_tree);
    assert(expr_node);
    assert(solv_tree);

fprintf(LogFile, "start of TakeDifferential():\n");
fprintf(stderr, "watawatawata\n");
DIFF_DUMP(solv_tree);
DIFF_DUMP(expr_tree);

    if (expr_node->type == NUM)
    {
        return NewNode(solv_tree, NUM, {.num = 0}, NULL, NULL);
    }

    else if (expr_node->type == VAR)
    {
        return NewNode(solv_tree, NUM, {.num = 1}, NULL, NULL);
    }


    Node *res_node = NULL;

    if (expr_node->type == CHANGE)
    {
fprintf(stderr, "in change diff\n");
        Change *cur_change = expr_node->val.change;

        char tex_cur_change_name[TEX_CHANGE_NAME_LEN] = {};
        GetTexChangedVarName(cur_change, tex_cur_change_name);
        fprintf(OutputFile, "Lets take a derivative of %s\\newline\n", tex_cur_change_name);
        
        int     diff_change_deriv_num = cur_change->derivative_num + 1;
        int     diff_change_name      = cur_change->name;
        Change *diff_change = &(solv_tree->changed_vars.data[solv_tree->changed_vars.size++]);

        diff_change->name           = (char) diff_change_name;
        diff_change->derivative_num = diff_change_deriv_num;
        
        diff_change->target_node = TakeDifferential(expr_tree, cur_change->target_node, solv_tree);

        char tex_diff[TEX_EXPRESSION_LEN] = {};
        GetTexTreeData(diff_change->target_node, tex_diff, false);

        char tex_change_name[TEX_CHANGE_NAME_LEN] = {};
        GetTexChangedVarName(diff_change, tex_change_name);
fprintf(stderr, "in chang simpl\n");
        SimplifyExpr(solv_tree, diff_change->target_node);

        Node *new_node = NewNode(solv_tree, CHANGE, {.change = diff_change}, NULL, NULL);

        char res_change_name_tex[TEX_CHANGE_NAME_LEN] = {};
        GetTexChangedVarName(diff_change, res_change_name_tex);

        char res_diff_tex[TEX_EXPRESSION_LEN] = {};
        GetTexTreeData(diff_change->target_node, res_diff_tex, false);

        fprintf(OutputFile, "So $%s = %s$\\newline\n\\newline\n", res_change_name_tex, res_diff_tex);
DIFF_DUMP(expr_tree);

        res_node = new_node;
    }

    else if (expr_node->type == OP)
    {
        char differentiated_tex[TEX_EXPRESSION_LEN] = {};
        GetTexTreeData(expr_node, differentiated_tex, false);
        fprintf(OutputFile, "Trying to take a derivative of $%s$...\\newline\n\\newline\n", differentiated_tex);

        const Operation *cur_op = GetOperationByNode(expr_node);

        res_node = cur_op->diff_func(expr_tree, expr_node, solv_tree);
        
        SimplifyExpr(solv_tree, res_node);
        char res_diff_tex[TEX_EXPRESSION_LEN] = {};
        GetTexTreeData(res_node, res_diff_tex, false);

        fprintf(OutputFile, "Having counted the most obvious derivative, which the Soviet spermatozoa were actually able to calculate in their minds, we get:\n$(%s)'(x) = %s$\\newline\n\\newline\n", differentiated_tex, res_diff_tex);
    }

    return res_node;
}