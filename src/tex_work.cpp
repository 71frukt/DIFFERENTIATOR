#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "diff_tree.h"
#include "tex_work.h"
#include "operations.h"
#include "gnuplot.h"

extern FILE *OutputFile;
extern FILE *InputFile;

const char *OperationToTex(int node_op)
{
    for (size_t i = 0; i < OPERATIONS_NUM; i++)
    {
        if (node_op == Operations[i].num)
            return Operations[i].tex_code;
    }

    //fprintf(stderr, "unknown operation in OperationToTex() numbered %d\n", node_op);
    return NULL;
}

const char *GetStrTreeData(Node *start_node, char *dest_str, bool need_brackets, StrDataType type)
{
    assert(start_node);
    assert(dest_str);

    const char *open_arg_bracket  = (type == TEX ? "{" : "(");
    const char *close_arg_bracket = (type == TEX ? "}" : ")");

    const char *open_expr_bracket  = (type == TEX ? "\\left(" : "(");
    const char *close_expr_bracket = (type == TEX ? "\\right)" : ")");

    char node_val_str[LABEL_LENGTH] = {};

    if (start_node->type != CHANGE)
        NodeValToStr(start_node, node_val_str);
    
    else
        GetTexChangedVarName(start_node->val.change, node_val_str);
    
    bool param1_brackets = false;
    bool param2_brackets = false;
    ParamsNeedBrackets(start_node, &param1_brackets, &param2_brackets);

    if (need_brackets)
        sprintf(dest_str + strlen(dest_str), "%s", open_expr_bracket);

    if (start_node->type != OP)
        sprintf(dest_str + strlen(dest_str), "%s%s%s", open_arg_bracket, node_val_str, close_arg_bracket);

    else
    {
        const Operation *cur_op = GetOperationByNode(start_node);
        FuncEntryForm func_form = {};
        // const char *op_tex = OperationToTex(start_node->val.op);

        const char *op_tex = NULL;

        if (type == TEX)
        {
            op_tex    = cur_op->tex_code;
            func_form = cur_op->tex_form;
        }

        else if (type == GNUPLOT)
        {
            op_tex    = cur_op->symbol;
            func_form = cur_op->life_form;
        }



        if (func_form == PREFIX)
        {
            sprintf(dest_str + strlen(dest_str), " %s ", op_tex);

            sprintf(dest_str + strlen(dest_str), "%s", open_arg_bracket); 
            GetStrTreeData(start_node->left,  dest_str + strlen(dest_str), param1_brackets, type);
            sprintf(dest_str + strlen(dest_str), "%s ", close_arg_bracket);

            if (cur_op->type == BINARY)
            {
                sprintf(dest_str + strlen(dest_str), "%s", open_arg_bracket); 
                GetStrTreeData(start_node->right, dest_str + strlen(dest_str), param2_brackets, type);
                sprintf(dest_str + strlen(dest_str), "%s", close_arg_bracket);
            }
        }

        else
        {
            sprintf(dest_str + strlen(dest_str), "%s", open_arg_bracket); 

            GetStrTreeData(start_node->left, dest_str, param1_brackets, type);

            sprintf(dest_str + strlen(dest_str), " %s ", op_tex);

            GetStrTreeData(start_node->right, dest_str + strlen(dest_str), param2_brackets, type);

            sprintf(dest_str + strlen(dest_str), "%s", close_arg_bracket);
        }    
    }

    if (need_brackets)
        sprintf(dest_str + strlen(dest_str), "%s", close_expr_bracket);  

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

    fprintf(OutputFile, "\\documentclass{article}                   \n"
                        "\\usepackage[utf8]{inputenc}               \n"
                        "\\usepackage[T2A]{fontenc}                 \n"
                        "\\usepackage{amsmath}                      \n"
                        "\\usepackage{amssymb}                      \n"
                        "\\usepackage{graphicx}                     \n"
                        "\\usepackage{float}                    \n\n\n"
                        "\\begin{document}                          \n");

    atexit(CloseOutputFile);

    return OutputFile;
}

FILE *GetInputFile(const int argc, const char *argv[])
{
    if (argc < 3)
        InputFile = fopen(BASE_INPUT_FILE_NAME, "r");
    
    else
        InputFile = fopen(argv[1], "w");

    atexit(CloseInputFile);

    return InputFile;
}

void PrintChangedVarsTex(Tree *tree, FILE *output_file)
{
    assert(tree);
    assert(output_file);

    ChangedVars *changed_vars = &tree->changed_vars;

    // for (int i = (int) changed_vars->size - 1; i >= 0; i--)
    // {
    for (size_t i = 0; i < changed_vars->size; i++)
    {
        Change *cur_change = &(changed_vars->data[i]);

        if (cur_change->target_node == NULL)
            continue;

        char tex_change_name[TEX_CHANGE_NAME_LEN] = {};
        GetTexChangedVarName(cur_change, tex_change_name);

        char tex_subtree[STR_EXPRESSION_LEN] = {};
        GetStrTreeData(cur_change->target_node, tex_subtree, false, TEX);

        fprintf(output_file, "$%s = %s$\n\\newline\\newline\n", tex_change_name, tex_subtree);
    }
    
    fprintf(output_file, "\\newline\n");
}

void DrawChart(FILE *dest_file, char *chart_file_name)
{
    fprintf(dest_file, "\\begin{figure}[H]                                              \n"
                       "     \\centering                                                \n"
                       "     \\includegraphics[width=0.8\\textwidth]{%s}                \n"
                       " \\end{figure}                                                  \n", chart_file_name);
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

void CloseInputFile()
{
    fclose(InputFile);
}