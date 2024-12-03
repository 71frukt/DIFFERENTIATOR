#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "diff_tree.h"
#include "tex_work.h"
#include "operations.h"

extern FILE *OutputFile;

const char *OperationToTex(int node_op)
{
    for (size_t i = 0; i < OPERATIONS_NUM; i++)
    {
        if (node_op == Operations[i].num)
            return Operations[i].tex_code;
    }

    fprintf(stderr, "unknown operation in OperationToTex() numbered %d\n", node_op);    // если не нашли
    return NULL;
}

const char *GetTexTreeData(Node *start_node, char *dest_str, bool need_brackets)
{
    assert(start_node);

    char node_val_str[LABEL_LENGTH] = {};

    if (start_node->type != CHANGE)
        NodeValToStr(start_node, node_val_str);
    
    else
        GetTexChangedVarName(start_node->val.change, node_val_str);
    
    bool param1_brackets = false;
    bool param2_brackets = false;
    ParamsNeedBrackets(start_node, &param1_brackets, &param2_brackets);

    IN_BRACKETS (need_brackets, dest_str, 

    if (start_node->type != OP)
        sprintf(dest_str + strlen(dest_str), "{%s}", node_val_str);

    else
    {
        const Operation *cur_op = GetOperationByNode(start_node);

        const char *op_tex = OperationToTex(start_node->val.op);
    
        if (cur_op->tex_form == PREFIX)
        {
            sprintf(dest_str + strlen(dest_str), " %s ", op_tex);

            sprintf(dest_str + strlen(dest_str), "{"); 
            GetTexTreeData(start_node->left,  dest_str + strlen(dest_str), param1_brackets);
            sprintf(dest_str + strlen(dest_str), "} ");

            if (cur_op->type == BINARY)
            {
                sprintf(dest_str + strlen(dest_str), "{"); 
                GetTexTreeData(start_node->right, dest_str + strlen(dest_str), param2_brackets);
                sprintf(dest_str + strlen(dest_str), "}");
            }
        }

        else
        {
            sprintf(dest_str + strlen(dest_str), "{"); 

            GetTexTreeData(start_node->left, dest_str, param1_brackets);

            sprintf(dest_str + strlen(dest_str), " %s ", op_tex);

            GetTexTreeData(start_node->right, dest_str + strlen(dest_str), param2_brackets);

            sprintf(dest_str + strlen(dest_str), "}");
        }    
    }

    );

    return dest_str;
}

void ParamsNeedBrackets(Node *op_node, bool *param_1, bool *param_2)
{
    if (op_node->type != OP)
        return;

    if (op_node->val.op == ADD)
    {
        Node *arg2 = op_node->right;

        if (arg2->type == NUM && arg2->val.num < 0)
            *param_2 = true;

        *param_1 = false;
    }

    else if (op_node->val.op == SUB)
    {
        if (op_node->right->type == OP && (op_node->right->val.op == ADD || op_node->right->val.op == SUB))
            *param_2 = true;

        *param_1 = false;
    }

    else if (op_node->val.op == DEG)
    {
        if (op_node->left->type == OP || (op_node->left->type == NUM && op_node->left->val.num < 0))
            *param_1 = true;
        else 
            *param_1 = false;

        *param_2 = false;
    }

    else if (op_node->val.op == MUL)
    {
        if (op_node->left->type == OP && (op_node->left->val.op == ADD   || op_node->left->val.op == SUB))
            *param_1 = true;

        if (op_node->right->type == OP && (op_node->right->val.op == ADD || op_node->right->val.op == SUB))
            *param_2 = true;

        if (op_node->left->type == NUM  && op_node->left->val.num < 0)
            *param_1 = true;

        if (op_node->right->type == NUM && op_node->right->val.num < 0)
            *param_2 = true; 
    }

    else if (IsTrigonometric(op_node->val.op))
    {
        if (op_node->left->type == OP)
            *param_1 = true;

        if (op_node->left->type == NUM && op_node->left->val.num < 0)
            *param_1 = true;
    }

    else if (op_node->val.op == DIF)
    {
        if (op_node->left->type == OP)
            *param_1 = true;
    }

    else
    {
        *param_1 = false;
        *param_2 = false;
    }
}

bool IsTrigonometric(int op)
{
    if (op == SIN || op == COS || op == TAN)
        return true;
    
    else
        return false;
}

FILE *GetOutputFile(const int argc, const char *argv[])
{
    if (argc < 2)
        OutputFile = fopen(TEX_FOLDER BASE_OUTPUT_FILE_NAME, "w");
    
    else
        OutputFile = fopen(argv[1], "w");

    setvbuf(OutputFile, NULL, _IONBF, 0);

    fprintf(OutputFile, "\\documentclass[a4paper, 12pt]{article}    \n"

                        "\\usepackage[utf8x]{inputenc}              \n"
                        "\\usepackage[english,russian]{babel}       \n"
                        "\\usepackage{cmap}                         \n"
                        "\\begin{document}                          \n"); 

    atexit(CloseOutputFile);

    return OutputFile;
}

void PrintChangedVarsTex(Tree *tree, FILE *output_file)
{
    assert(tree);
    assert(output_file);

    ChangedVars *changed_vars = &tree->changed_vars;

    for (size_t derivative_num = 0; derivative_num < CHANGED_VARS_DERIVATIVE_NUM; derivative_num++)
    {
        for (int var_num = (int) changed_vars->size - 1; var_num >= 0; var_num--)
        {
            Change *cur_change = &(changed_vars->data[derivative_num][var_num]);

            if (cur_change->target_node == NULL)
                continue;

            char tex_change_name[TEX_CHANGE_NAME_LEN] = {};
            GetTexChangedVarName(cur_change, tex_change_name);

            char tex_subtree[TEX_EXPRESSION_LEN] = {};
            GetTexTreeData(cur_change->target_node, tex_subtree, false);

            fprintf(output_file, "$%s = %s$\n\n", tex_change_name, tex_subtree);
        }
    }
}

void GetTexChangedVarName(Change *change, char *res_name)
{
    if (change->derivative_num == 0)
        sprintf(res_name, "%c", change->name);
    
    else if (change->derivative_num == 1)
        sprintf(res_name, "%c_x'", change->name);

    else if (change->derivative_num == 2)
        sprintf(res_name, "%c_x''", change->name);

    else
        sprintf(res_name, "%c_x^(%d)", change->name, change->derivative_num);
}

void CloseOutputFile()
{
    fprintf(OutputFile, "\\end{document}\n");
    fclose(OutputFile);
}