#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "diff_tree.h"

const char *OperationToTex(int node_op)
{
    for (size_t i = 0; i < OperationsArr.size; i++)
    {
        if (node_op == OperationsArr.data[i].num)
            return OperationsArr.data[i].tex_code;
    }

    fprintf(stderr, "unknown operation in OperationToTex() numbered %d\n", node_op);    // если не нашли
    return NULL;
}

const char *GetTexTreeData(Node *start_node, char *dest_str, bool need_brackets)
{
    assert(start_node);

    char node_val_str[LABEL_LENGTH] = {};
    NodeValToStr(start_node->value, start_node->type, node_val_str);

    if (start_node->type != OP)
        sprintf(dest_str + strlen(dest_str), "%s", node_val_str);

    else
    {
        const char *op_tex = OperationToTex((int) start_node->value);

        bool param1_brackets = true;
        bool param2_brackets = true;
        ParamsNeedBrackets(start_node, &param1_brackets, &param2_brackets);

        if (IsPrefixOperation((int) start_node->value))
        {
            sprintf(dest_str + strlen(dest_str), " %s ", op_tex);

            sprintf(dest_str + strlen(dest_str), "{"); 
            GetTexTreeData(start_node->left,  dest_str + strlen(dest_str), param1_brackets);
            sprintf(dest_str + strlen(dest_str), "} {");

            GetTexTreeData(start_node->right, dest_str + strlen(dest_str), param2_brackets);
            sprintf(dest_str + strlen(dest_str), "}");
        }

        else
        {
            if (need_brackets) 
                sprintf(dest_str + strlen(dest_str), "\\left(");

            sprintf(dest_str + strlen(dest_str), "{"); 

            GetTexTreeData(start_node->left, dest_str, param1_brackets);

            sprintf(dest_str + strlen(dest_str), " %s ", op_tex);

            GetTexTreeData(start_node->right, dest_str + strlen(dest_str), param2_brackets);

            sprintf(dest_str + strlen(dest_str), "}");

            if (need_brackets)
                sprintf(dest_str + strlen(dest_str), "\\right)");
        }    
    }

    return dest_str;
}

bool IsPrefixOperation(int op)
{
    for (size_t i = 0; i < OperationsArr.size; i++)
    {
        if (op == OperationsArr.data[i].num)
        {
            if (OperationsArr.data[i].form == IS_PREFIX)
                return true;
            else
                return false;
        }
    }

    fprintf(stderr, "unknown operation in IsPrefixOperation() numbered %d\n", op);
    return false;
}

void ParamsNeedBrackets(Node *op_node, bool *param_1, bool *param_2)
{
    if ((int) op_node->value == DEG)
    {
        *param_1 = true;
        *param_2 = false;
    }

    else if ((int) op_node->value == MUL)
    {
        if (op_node->type == OP && ((int) op_node->left->value  == ADD || (int) op_node->left->value  == SUB))
            *param_1 = true;

        if (op_node->type == OP && ((int) op_node->right->value == ADD || (int) op_node->right->value == SUB))
            *param_2 = true;

        else
        {
            *param_1 = false;
            *param_2 = false;
        }
    }

    else
    {
        *param_1 = false;
        *param_2 = false;
    }
}