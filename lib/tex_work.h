#ifndef TEX_WORK
#define TEX_WORK

#include <stdio.h>

#include "diff_tree.h"

#define TEX_FOLDER             "tex/"
#define BASE_OUTPUT_FILE_NAME  "dest_file.tex"

const size_t TEX_EXPRESSION_LEN  = 1000;
const size_t TEX_CHANGE_NAME_LEN = 30;

const char *OperationToTex     (int node_op);
const char *GetTexTreeData     (Node *start_node, char *dest_str, bool need_brackets);
void        ParamsNeedBrackets (Node *op_node, bool *param_1, bool *param_2);

FILE *GetOutputFile   (const int argc, const char *argv[]);
void  CloseOutputFile ();

void PrintChangedVarsTex  (Tree *tree, FILE *output_file);
void GetTexChangedVarName (Change *change, char *res_name);

#define IN_BRACKETS(need_brackets, dest_str, ...)               \
{                                                               \
    if (need_brackets)                                          \
        sprintf(dest_str + strlen(dest_str), "\\left(");        \
                                                                \
    __VA_ARGS__;                                                \
                                                                \
    if (need_brackets)                                          \
        sprintf(dest_str + strlen(dest_str), "\\right)");       \
}                                                           

#endif