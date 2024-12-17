#ifndef TEX_WORK
#define TEX_WORK

#include <stdio.h>

#include "diff_tree.h"

#define TEX_FOLDER             "tex/"
#define BASE_OUTPUT_FILE_NAME  "dest_file.tex"

enum StrDataType
{
    TEX,
    GNUPLOT
};

const size_t STR_EXPRESSION_LEN  = 25000;
const size_t TEX_CHANGE_NAME_LEN = 30;

const char *OperationToTex     (int node_op);
const char *GetStrTreeData     (Node *start_node, char *dest_str, bool need_brackets, StrDataType type);
void        ParamsNeedBrackets (Node *op_node, bool *param_1, bool *param_2);

FILE *GetOutputFile (const int argc, const char *argv[]);
FILE *GetInputFile  (const int argc, const char *argv[]);

void  CloseOutputFile ();
void  CloseInputFile  ();

void PrintChangedVarsTex  (Tree *tree, FILE *output_file);
void DrawChart            (FILE *dest_file, char *chart_file_name);
void GetTexChangedVarName (Change *change, char *res_name);                                                       

#endif