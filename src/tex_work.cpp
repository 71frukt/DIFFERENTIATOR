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
    NodeValToStr(start_node, node_val_str);
    
    bool param1_brackets = false;
    bool param2_brackets = false;
    ParamsNeedBrackets(start_node, &param1_brackets, &param2_brackets);

    IN_BRACKETS (need_brackets, dest_str, 

    if (start_node->type != OP)
        sprintf(dest_str + strlen(dest_str), "{%s}", node_val_str);

    else
    {
        const Operation *cur_op = GetOperationByNode(start_node);

        const char *op_tex = OperationToTex((int) start_node->value);
    
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

    if (op_node->value == ADD)
    {
        Node *arg2 = op_node->right;

        if (arg2->type == NUM && arg2->value < 0)
            *param_2 = true;

        *param_1 = false;
    }

    else if (op_node->value == SUB)
    {
        if (op_node->right->type == OP && (op_node->right->value == ADD || op_node->right->value == SUB))
            *param_2 = true;

        *param_1 = false;
    }

    else if (op_node->value == DEG)
    {
        if (op_node->left->type == OP || (op_node->left->type == NUM && op_node->left->value < 0))
            *param_1 = true;
        else 
            *param_1 = false;

        *param_2 = false;
    }

    else if (op_node->value == MUL)
    {
        if (op_node->left->type == OP && (op_node->left->value == ADD || op_node->left->value == SUB))
            *param_1 = true;

        if (op_node->right->type == OP && (op_node->right->value == ADD || op_node->right->value == SUB))
            *param_2 = true;

        if (op_node->left->type == NUM && op_node->left->value < 0)
            *param_1 = true;

        if (op_node->right->type == NUM && op_node->right->value < 0)
            *param_2 = true; 
    }

    else if (IsTrigonometric(op_node->value))
    {
        if (op_node->left->type == OP)
            *param_1 = true;

        if (op_node->left->type == NUM && op_node->left->value < 0)
            *param_1 = true;
    }

    else if (op_node->value == DIF)
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

// void PrintChangedVarsTex(Tree *tree, FILE *output_file)
// {
//     ChangedVars *changed_vars = &tree->changed_vars;

//     for (size_t derivative_num = 0; derivative_num < CHANGED_VARS_DERIVATIVE_NUM; derivative_num++)
//     {
//         for (size_t var_num = 0; var_num < changed_vars->size; var_num++)
//         {
//             if (changed_vars->data[derivative_num][var_num] == NULL)
//                 continue;

            
//         }
//     }
// }

// void GetChangedVarTexName(ChangedVars *changed_vars, TreeElem_t var_name, char *res_name)
// {
//     for (size_t i = 0; i < CHANGED_VARS_DERIVATIVE_NUM; i++)
//     {

//     }
// }

void CloseOutputFile()
{
    fprintf(OutputFile, "\\end{document}\n");
    fclose(OutputFile);
}