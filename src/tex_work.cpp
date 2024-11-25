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
    NodeValToStr(start_node->value, start_node->type, node_val_str);
    
    bool param1_brackets = false;
    bool param2_brackets = false;
    ParamsNeedBrackets(start_node, &param1_brackets, &param2_brackets);

    IN_BRACKETS (need_brackets, dest_str, 

    if (start_node->type != OP)
        sprintf(dest_str + strlen(dest_str), "%s", node_val_str);

    else
    {
        const Operation *cur_op = GetOperationByNum(start_node->value);

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
            // if (need_brackets) 
            //     sprintf(dest_str + strlen(dest_str), "\\left(");

            sprintf(dest_str + strlen(dest_str), "{"); 

            GetTexTreeData(start_node->left, dest_str, param1_brackets);

            sprintf(dest_str + strlen(dest_str), " %s ", op_tex);

            GetTexTreeData(start_node->right, dest_str + strlen(dest_str), param2_brackets);

            sprintf(dest_str + strlen(dest_str), "}");

            // if (need_brackets)
            //     sprintf(dest_str + strlen(dest_str), "\\right)");
        }    
    }

    );

    return dest_str;
}

void ParamsNeedBrackets(Node *op_node, bool *param_1, bool *param_2)
{
    if (op_node->type != OP)
        return;

    if ((int) op_node->value == DEG)
    {
        if (op_node->left->type == OP || (op_node->left->type == NUM && op_node->left->value < 0))
            *param_1 = true;
        else 
            *param_1 = false;

        *param_2 = false;
    }

    else if ((int) op_node->value == MUL)
    {
        if ((int) op_node->left->type == OP && ((int) op_node->left->value == ADD || (int) op_node->left->value == SUB))
            *param_1 = true;

        if ((int) op_node->right->type == OP && ((int) op_node->right->value == ADD || (int) op_node->right->value == SUB))
            *param_2 = true;

        if ((int) op_node->left->type == NUM && op_node->left->value < 0)
            *param_1 = true;

        if ((int) op_node->right->type == NUM && op_node->right->value < 0)
            *param_2 = true; 

        // fprintf(stderr, "1 = %d, 2 = %d\n", *param_1, *param_2);
    }

    else if (IsTrigonometric((int) op_node->value))
    {
        if (op_node->left->type == OP)
            *param_1 = true;

        if (op_node->left->type == NUM && op_node->left->value < 0)
            *param_1 = true;
    }

    else if ((int) op_node->value == DIF)
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

    fprintf(OutputFile, "\\documentclass[a4paper, 12pt]{article}    \n"

                        "\\usepackage[utf8x]{inputenc}              \n"
                        "\\usepackage[english,russian]{babel}       \n"
                        "\\usepackage{cmap}                         \n"
                        "\\begin{document}                          \n"); 

    atexit(CloseOutputFile);

    return OutputFile;
}

void CloseOutputFile()
{
    fprintf(OutputFile, "\\end{document}\n");
    fclose(OutputFile);
}